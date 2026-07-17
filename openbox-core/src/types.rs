//! Common Types for Openbox Core

use serde::{Deserialize, Serialize};
use std::fmt;

/// Window handle (platform-agnostic)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum WindowHandle {
    /// X11 window ID
    X11(u64),
    /// Wayland surface ID
    Wayland(u64),
    /// Generic handle for testing
    Generic(u64),
}

impl WindowHandle {
    /// Get as X11 window ID if applicable
    pub fn as_x11(&self) -> Option<u64> {
        match self {
            WindowHandle::X11(id) => Some(*id),
            _ => None,
        }
    }

    /// Get as Wayland surface ID if applicable
    pub fn as_wayland(&self) -> Option<u64> {
        match self {
            WindowHandle::Wayland(id) => Some(*id),
            _ => None,
        }
    }
}

impl fmt::Display for WindowHandle {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            WindowHandle::X11(id) => write!(f, "X11:0x{:x}", id),
            WindowHandle::Wayland(id) => write!(f, "WL:{}", id),
            WindowHandle::Generic(id) => write!(f, "WIN:{}", id),
        }
    }
}

/// Window geometry
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq, Serialize, Deserialize)]
pub struct Geometry {
    pub x: i32,
    pub y: i32,
    pub width: u32,
    pub height: u32,
}

impl Geometry {
    /// Create a new geometry
    pub fn new(x: i32, y: i32, width: u32, height: u32) -> Self {
        Self { x, y, width, height }
    }

    /// Check if point is inside geometry
    pub fn contains(&self, px: i32, py: i32) -> bool {
        px >= self.x
            && px < self.x + self.width as i32
            && py >= self.y
            && py < self.y + self.height as i32
    }

    /// Get center point
    pub fn center(&self) -> (i32, i32) {
        (
            self.x + self.width as i32 / 2,
            self.y + self.height as i32 / 2,
        )
    }

    /// Check if this geometry intersects with another
    pub fn intersects(&self, other: &Geometry) -> bool {
        !(self.x + self.width as i32 <= other.x
            || other.x + other.width as i32 <= self.x
            || self.y + self.height as i32 <= other.y
            || other.y + other.height as i32 <= self.y)
    }

    /// Get intersection area
    pub fn intersection(&self, other: &Geometry) -> Option<Geometry> {
        let x1 = self.x.max(other.x);
        let y1 = self.y.max(other.y);
        let x2 = (self.x + self.width as i32).min(other.x + other.width as i32);
        let y2 = (self.y + self.height as i32).min(other.y + other.height as i32);

        if x1 < x2 && y1 < y2 {
            Some(Geometry::new(
                x1,
                y1,
                (x2 - x1) as u32,
                (y2 - y1) as u32,
            ))
        } else {
            None
        }
    }
}

/// Rectangle (simpler version of geometry)
pub type Rect = Geometry;

/// Position
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq, Serialize, Deserialize)]
pub struct Position {
    pub x: i32,
    pub y: i32,
}

/// Size
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq, Serialize, Deserialize)]
pub struct Size {
    pub width: u32,
    pub height: u32,
}

/// Direction for window operations
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Direction {
    North,
    South,
    East,
    West,
    NorthEast,
    NorthWest,
    SouthEast,
    SouthWest,
    Up,
    Down,
    Left,
    Right,
}

/// Desktop/workspace identifier
pub type DesktopId = i32;

/// Event types
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum EventType {
    /// Window created
    Create,
    /// Window destroyed
    Destroy,
    /// Window mapped (shown)
    Map,
    /// Window unmapped (hidden)
    Unmap,
    /// Window configured (resized/moved)
    Configure,
    /// Property changed
    Property,
    /// Focus in
    FocusIn,
    /// Focus out
    FocusOut,
    /// Mouse enter
    Enter,
    /// Mouse leave
    Leave,
    /// Mouse button press
    ButtonPress,
    /// Mouse button release
    ButtonRelease,
    /// Mouse motion
    Motion,
    /// Key press
    KeyPress,
    /// Key release
    KeyRelease,
    /// Desktop switch
    DesktopSwitch,
    /// Screen configuration change
    ScreenChange,
    /// Client message
    ClientMessage,
}

/// Key modifiers
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct Modifiers {
    pub shift: bool,
    pub control: bool,
    pub alt: bool,
    pub super_key: bool,
    pub hyper: bool,
    pub meta: bool,
}

impl Modifiers {
    /// Empty modifiers
    pub fn none() -> Self {
        Self {
            shift: false,
            control: false,
            alt: false,
            super_key: false,
            hyper: false,
            meta: false,
        }
    }

    /// Check if any modifier is pressed
    pub fn any(&self) -> bool {
        self.shift || self.control || self.alt || self.super_key || self.hyper || self.meta
    }
}

/// Mouse button
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum MouseButton {
    Left,
    Middle,
    Right,
    ScrollUp,
    ScrollDown,
    ScrollLeft,
    ScrollRight,
    Button(u8),
}

/// Key symbol
pub type KeySym = u32;

/// Key code
pub type KeyCode = u8;

/// Atom/property name
pub type Atom = String;

/// Time stamp (milliseconds)
pub type TimeStamp = u64;

/// Client message data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum ClientMessageData {
    Bytes([u8; 20]),
    Shorts([u16; 10]),
    Longs([u64; 5]),
}

/// Screen information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ScreenInfo {
    pub id: u32,
    pub width: u32,
    pub height: u32,
    pub x: i32,
    pub y: i32,
    pub primary: bool,
    pub dpi: f64,
}

/// Monitor/output information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MonitorInfo {
    pub id: String,
    pub name: String,
    pub width: u32,
    pub height: u32,
    pub x: i32,
    pub y: i32,
    pub primary: bool,
    pub scale_factor: f64,
}

/// Strut/reserved space
#[derive(Debug, Clone, Copy, Default, Serialize, Deserialize)]
pub struct Strut {
    pub left: u32,
    pub right: u32,
    pub top: u32,
    pub bottom: u32,
}

/// Window gravity
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum WindowGravity {
    NorthWest,
    North,
    NorthEast,
    West,
    Center,
    East,
    SouthWest,
    South,
    SouthEast,
    Static,
}

/// Window decorations
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Decoration {
    All,
    Border,
    Titlebar,
    None,
}

/// Window placement policy
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum PlacementPolicy {
    Smart,
    UnderMouse,
    Center,
    Cascade,
    Manual,
}

/// Startup notification state
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct StartupNotification {
    pub id: String,
    pub name: String,
    pub icon: Option<String>,
    pub wmclass: Option<String>,
    pub desktop: Option<DesktopId>,
    pub timestamp: TimeStamp,
}

/// Session state
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum SessionState {
    Normal,
    Minimized,
    Maximized,
    MaximizedVert,
    MaximizedHorz,
    Fullscreen,
    Shaded,
    Iconic,
}

/// EWMH window type
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum EwmhWindowType {
    Normal,
    Desktop,
    Dock,
    Toolbar,
    Menu,
    Utility,
    Splash,
    Dialog,
    Notification,
}

/// Window protocol
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum WindowProtocol {
    DeleteWindow,
    TakeFocus,
    Ping,
    SyncRequest,
    ContextHelp,
    NetWmPing,
    NetWmSyncRequest,
}

/// Frame extents
#[derive(Debug, Clone, Copy, Default, Serialize, Deserialize)]
pub struct Extents {
    pub left: i32,
    pub right: i32,
    pub top: i32,
    pub bottom: i32,
}

/// Border width
pub type BorderWidth = u32;

/// Padding
#[derive(Debug, Clone, Copy, Default, Serialize, Deserialize)]
pub struct Padding {
    pub top: u32,
    pub bottom: u32,
    pub left: u32,
    pub right: u32,
}

/// Margin
pub type Margin = Padding;

/// Color (RGBA)
#[derive(Debug, Clone, Copy, Default, PartialEq, Serialize, Deserialize)]
pub struct Color {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub a: u8,
}

impl Color {
    /// Create a color from RGB values
    pub fn rgb(r: u8, g: u8, b: u8) -> Self {
        Self { r, g, b, a: 255 }
    }

    /// Create a color from RGBA values
    pub fn rgba(r: u8, g: u8, b: u8, a: u8) -> Self {
        Self { r, g, b, a }
    }

    /// Parse from hex string (#RRGGBB or #RRGGBBAA)
    pub fn from_hex(hex: &str) -> Option<Self> {
        let hex = hex.trim_start_matches('#');
        match hex.len() {
            6 => {
                let r = u8::from_str_radix(&hex[0..2], 16).ok()?;
                let g = u8::from_str_radix(&hex[2..4], 16).ok()?;
                let b = u8::from_str_radix(&hex[4..6], 16).ok()?;
                Some(Self::rgb(r, g, b))
            }
            8 => {
                let r = u8::from_str_radix(&hex[0..2], 16).ok()?;
                let g = u8::from_str_radix(&hex[2..4], 16).ok()?;
                let b = u8::from_str_radix(&hex[4..6], 16).ok()?;
                let a = u8::from_str_radix(&hex[6..8], 16).ok()?;
                Some(Self::rgba(r, g, b, a))
            }
            _ => None,
        }
    }
}

/// Font specification
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FontSpec {
    pub family: String,
    pub size: f64,
    pub weight: FontWeight,
    pub style: FontStyle,
}

/// Font weight
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum FontWeight {
    Normal,
    Bold,
    Light,
    Medium,
    Heavy,
}

/// Font style
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum FontStyle {
    Normal,
    Italic,
    Oblique,
}

/// Action types
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Action {
    Focus,
    Raise,
    Lower,
    Close,
    Iconify,
    Maximize,
    MaximizeVert,
    MaximizeHorz,
    Shade,
    Unshade,
    ToggleMaximize,
    ToggleShade,
    ToggleDecorations,
    Move,
    Resize,
    MoveToCenter,
    MoveToCorner(Direction),
    GrowToEdge(Direction),
    SendToDesktop(DesktopId),
    GoToDesktop(DesktopId),
    NextDesktop,
    PrevDesktop,
    Execute(String),
    ShowMenu(String),
    Reconfigure,
    Restart,
    Exit,
    Chain(Vec<Action>),
}

/// Key binding
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct KeyBinding {
    pub key: KeySym,
    pub modifiers: Modifiers,
    pub chain: Option<Vec<KeyBinding>>,
    pub action: Action,
}

/// Mouse binding
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MouseBinding {
    pub button: MouseButton,
    pub modifiers: Modifiers,
    pub context: MouseContext,
    pub action: Action,
}

/// Mouse binding context
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum MouseContext {
    Frame,
    Titlebar,
    Top,
    Bottom,
    Left,
    Right,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Client,
    Icon,
    AllDesktops,
    Shade,
    Iconify,
    Maximize,
    Close,
    Desktop,
    Root,
}

/// Theme specification
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ThemeSpec {
    pub name: String,
    pub title_layout: String,
    pub keep_border: bool,
    pub animate_iconify: bool,
    pub fonts: ThemeFonts,
}

/// Theme fonts
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ThemeFonts {
    pub active_window: FontSpec,
    pub inactive_window: FontSpec,
    pub menu_header: FontSpec,
    pub menu_item: FontSpec,
    pub osd_active: FontSpec,
    pub osd_inactive: FontSpec,
}

/// Error types for window operations
#[derive(Debug, Clone)]
pub enum WindowError {
    InvalidHandle,
    InvalidGeometry,
    InvalidState,
    ProtocolNotSupported,
    Platform(String),
}

impl fmt::Display for WindowError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            WindowError::InvalidHandle => write!(f, "Invalid window handle"),
            WindowError::InvalidGeometry => write!(f, "Invalid geometry"),
            WindowError::InvalidState => write!(f, "Invalid window state"),
            WindowError::ProtocolNotSupported => write!(f, "Protocol not supported"),
            WindowError::Platform(s) => write!(f, "Platform error: {}", s),
        }
    }
}

impl std::error::Error for WindowError {}
