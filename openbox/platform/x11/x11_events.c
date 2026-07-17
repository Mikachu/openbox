/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   x11_events.c - X11 Event Translation for PAL
   Copyright (c) 2026  Openbox Development Team

   Translates X11 events to platform-agnostic events.
*/

#include "x11_backend.h"
#include "obt/keyboard.h"

/* Button mapping from X11 to platform */
static ObButtonMask x11_button_to_platform(guint button)
{
    switch (button) {
        case Button1: return OB_BUTTON_LEFT;
        case Button2: return OB_BUTTON_MIDDLE;
        case Button3: return OB_BUTTON_RIGHT;
        case Button4: return OB_BUTTON_SCROLL_UP;
        case Button5: return OB_BUTTON_SCROLL_DOWN;
        case 6: return OB_BUTTON_SCROLL_LEFT;
        case 7: return OB_BUTTON_SCROLL_RIGHT;
        default: return OB_BUTTON_NONE;
    }
}

/* Modifier mapping from X11 to platform */
static ObModifierMask x11_state_to_modifiers(guint state)
{
    ObModifierMask mods = OB_MODIFIER_NONE;
    
    if (state & ShiftMask)   mods |= OB_MODIFIER_SHIFT;
    if (state & ControlMask) mods |= OB_MODIFIER_CTRL;
    if (state & Mod1Mask)    mods |= OB_MODIFIER_ALT;
    if (state & Mod4Mask)    mods |= OB_MODIFIER_SUPER;
    
    return mods;
}

/* X11 event type mapping */
static ObEventType x11_event_type(int xtype)
{
    switch (xtype) {
        case Expose:           return OB_EVENT_EXPOSE;
        case ConfigureNotify:  return OB_EVENT_CONFIGURE;
        case MapNotify:        return OB_EVENT_MAP;
        case UnmapNotify:      return OB_EVENT_UNMAP;
        case DestroyNotify:    return OB_EVENT_DESTROY;
        case FocusIn:          return OB_EVENT_FOCUS_IN;
        case FocusOut:         return OB_EVENT_FOCUS_OUT;
        case EnterNotify:      return OB_EVENT_ENTER;
        case LeaveNotify:      return OB_EVENT_LEAVE;
        case ButtonPress:      return OB_EVENT_BUTTON_PRESS;
        case ButtonRelease:    return OB_EVENT_BUTTON_RELEASE;
        case MotionNotify:     return OB_EVENT_MOTION;
        case KeyPress:         return OB_EVENT_KEY_PRESS;
        case KeyRelease:       return OB_EVENT_KEY_RELEASE;
        case PropertyNotify:   return OB_EVENT_PROPERTY;
        case ClientMessage:    return OB_EVENT_CLIENT_MESSAGE;
        default:               return OB_EVENT_UNKNOWN;
    }
}

gboolean x11_event_translate(XEvent *xevent, ObPlatformEvent *event)
{
    ObX11Display *d = x11_display_get();
    if (!d) return FALSE;
    
    event->type = x11_event_type(xevent->type);
    event->display = (ObPlatformDisplay *)d;
    event->native_event = xevent;
    
    /* Update last event time */
    Time xtime = CurrentTime;
    
    switch (xevent->type) {
        case Expose:
            event->data.expose.area.x = xevent->xexpose.x;
            event->data.expose.area.y = xevent->xexpose.y;
            event->data.expose.area.width = xevent->xexpose.width;
            event->data.expose.area.height = xevent->xexpose.height;
            event->data.expose.count = xevent->xexpose.count;
            xtime = xevent->xexpose.time;
            break;
            
        case ConfigureNotify:
            event->data.configure.x = xevent->xconfigure.x;
            event->data.configure.y = xevent->xconfigure.y;
            event->data.configure.width = xevent->xconfigure.width;
            event->data.configure.height = xevent->xconfigure.height;
            event->data.configure.border_width = xevent->xconfigure.border_width;
            event->data.configure.above = xevent->xconfigure.above != None;
            xtime = xevent->xconfigure.time;
            break;
            
        case ButtonPress:
        case ButtonRelease:
            event->data.button.x = xevent->xbutton.x;
            event->data.button.y = xevent->xbutton.y;
            event->data.button.button = x11_button_to_platform(xevent->xbutton.button);
            event->data.button.modifiers = x11_state_to_modifiers(xevent->xbutton.state);
            xtime = xevent->xbutton.time;
            break;
            
        case MotionNotify:
            event->data.motion.x = xevent->xmotion.x;
            event->data.motion.y = xevent->xmotion.y;
            event->data.motion.modifiers = x11_state_to_modifiers(xevent->xmotion.state);
            xtime = xevent->xmotion.time;
            break;
            
        case KeyPress:
        case KeyRelease:
            event->data.key.keycode = xevent->xkey.keycode;
            event->data.key.keysym = XKeycodeToKeysym(d->xdisplay, 
                                                        xevent->xkey.keycode, 0);
            event->data.key.modifiers = x11_state_to_modifiers(xevent->xkey.state);
            event->data.key.is_repeat = FALSE; /* X11 doesn't expose this easily */
            xtime = xevent->xkey.time;
            break;
            
        case PropertyNotify:
            event->data.property.name = g_strdup(XGetAtomName(d->xdisplay,
                                                              xevent->xproperty.atom));
            event->data.property.deleted = (xevent->xproperty.state == PropertyDelete);
            xtime = xevent->xproperty.time;
            break;
            
        case ClientMessage:
            event->data.client.message_type = g_strdup(XGetAtomName(d->xdisplay,
                                                                    xevent->xclient.message_type));
            for (int i = 0; i < 5; i++) {
                event->data.client.data[i] = xevent->xclient.data.l[i];
            }
            xtime = CurrentTime;
            break;
            
        case EnterNotify:
        case LeaveNotify:
            event->data.motion.x = xevent->xcrossing.x;
            event->data.motion.y = xevent->xcrossing.y;
            event->data.motion.modifiers = x11_state_to_modifiers(xevent->xcrossing.state);
            xtime = xevent->xcrossing.time;
            break;
            
        case FocusIn:
        case FocusOut:
            xtime = CurrentTime;
            break;
            
        case MapNotify:
        case UnmapNotify:
        case DestroyNotify:
            xtime = CurrentTime;
            break;
            
        default:
            /* Handle extension events */
#ifdef SHAPE
            if (d->has_shape && xevent->type == d->shape_event_base + ShapeNotify) {
                event->type = OB_EVENT_DAMAGE;
                xtime = CurrentTime;
            }
#endif
#ifdef XRANDR
            if (d->has_randr && xevent->type == d->randr_event_base + RRScreenChangeNotify) {
                event->type = OB_EVENT_OUTPUT_CHANGE;
                event->data.output.monitor = NULL; /* Will be filled by handler */
                event->data.output.connected = TRUE;
                xtime = CurrentTime;
            }
#endif
            break;
    }
    
    if (xtime != CurrentTime) {
        event->time = xtime;
        d->last_event_time = xtime;
    } else {
        event->time = d->last_event_time;
    }
    
    return event->type != OB_EVENT_UNKNOWN;
}

void x11_event_send(ObX11Window *window, XEvent *event)
{
    if (!window || !window->display) return;
    
    event->xany.display = window->display->xdisplay;
    event->xany.window = window->xwindow;
    
    XSendEvent(window->display->xdisplay, window->xwindow, 
               False, NoEventMask, event);
}

/* Poll for next event */
static gboolean x11_event_poll_impl(ObPlatformDisplay *display,
                                    ObPlatformEvent *event)
{
    ObX11Display *d = (ObX11Display *)display;
    
    /* Check queued events first */
    if (!g_queue_is_empty(d->event_queue)) {
        ObPlatformEvent *queued = g_queue_pop_head(d->event_queue);
        memcpy(event, queued, sizeof(ObPlatformEvent));
        g_free(queued);
        return TRUE;
    }
    
    /* Check for X events */
    if (XPending(d->xdisplay) > 0) {
        XEvent xevent;
        XNextEvent(d->xdisplay, &xevent);
        
        if (x11_event_translate(&xevent, event)) {
            return TRUE;
        }
    }
    
    return FALSE;
}

static void x11_event_free_impl(ObPlatformEvent *event)
{
    if (!event) return;
    
    /* Free platform-specific data */
    switch (event->type) {
        case OB_EVENT_PROPERTY:
            g_free(event->data.property.name);
            break;
        case OB_EVENT_CLIENT_MESSAGE:
            g_free(event->data.client.message_type);
            break;
        default:
            break;
    }
    
    /* Note: native_event is owned by X11, don't free */
}

const ObPlatformInterface *x11_backend_get_events_interface(void)
{
    static const ObPlatformInterface events_interface = {
        .event_poll = x11_event_poll_impl,
        .event_free = x11_event_free_impl,
    };
    
    return &events_interface;
}
