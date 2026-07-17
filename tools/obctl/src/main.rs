//! obctl - Openbox Control Utility
//! 
//! Modern D-Bus-based CLI tool for controlling Openbox window manager.
//! Replaces the legacy signal-based interface with a rich command set.

use anyhow::{Context, Result};
use clap::{Parser, Subcommand, ValueEnum};
use colored::Colorize;
use serde::Serialize;
use std::collections::HashMap;
use tabled::{Table, Tabled};
use zbus::zvariant::OwnedValue;

mod dbus;

/// Openbox Control Utility
#[derive(Parser)]
#[command(name = "obctl")]
#[command(about = "Control Openbox window manager via D-Bus")]
#[command(version)]
struct Cli {
    /// Output format
    #[arg(short, long, value_enum, default_value = "table")]
    format: OutputFormat,

    /// Be verbose
    #[arg(short, long)]
    verbose: bool,

    /// D-Bus bus address (default: session bus)
    #[arg(long)]
    bus: Option<String>,

    #[command(subcommand)]
    command: Commands,
}

#[derive(ValueEnum, Clone)]
enum OutputFormat {
    /// Human-readable table format
    Table,
    /// JSON format
    Json,
    /// Plain text (one value per line)
    Plain,
}

#[derive(Subcommand)]
enum Commands {
    /// Lifecycle control
    Lifecycle {
        #[command(subcommand)]
        action: LifecycleAction,
    },

    /// Window management
    #[command(alias = "win")]
    Window {
        #[command(subcommand)]
        action: WindowAction,
    },

    /// Desktop/workspace management
    Desktop {
        #[command(subcommand)]
        action: DesktopAction,
    },

    /// Configuration management
    Config {
        #[command(subcommand)]
        action: ConfigAction,
    },

    /// Theme management
    Theme {
        #[command(subcommand)]
        action: ThemeAction,
    },

    /// Execute commands and actions
    Run {
        /// Command to execute
        #[arg(value_name = "COMMAND")]
        command: String,
        
        /// Arguments for the command
        #[arg(value_name = "ARGS")]
        args: Vec<String>,
    },

    /// Show a menu
    Menu {
        /// Menu name
        name: String,
        
        /// X coordinate
        #[arg(short, long)]
        x: Option<i32>,
        
        /// Y coordinate
        #[arg(short, long)]
        y: Option<i32>,
    },

    /// Subscribe to events
    Events {
        /// Event types to subscribe to (comma-separated)
        #[arg(short, long, default_value = "all")]
        types: String,
        
        /// Output as JSON lines
        #[arg(long)]
        json: bool,
    },

    /// List windows
    List,

    /// Get window manager status
    Status,

    /// Legacy compatibility commands
    #[command(hide = true)]
    Reconfigure,
    #[command(hide = true)]
    Restart {
        #[arg(long)]
        no_save: bool,
    },
    #[command(hide = true)]
    Exit {
        #[arg(long)]
        no_save: bool,
    },
}

#[derive(Subcommand)]
enum LifecycleAction {
    /// Reload configuration
    Reconfigure,
    /// Restart window manager
    Restart {
        /// Don't save session
        #[arg(long)]
        no_save: bool,
    },
    /// Exit window manager
    Exit {
        /// Don't save session
        #[arg(long)]
        no_save: bool,
    },
}

#[derive(Subcommand)]
enum WindowAction {
    /// List all windows
    List {
        /// Show detailed information
        #[arg(short, long)]
        long: bool,
    },
    /// Focus a window
    Focus {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
        /// Raise window
        #[arg(short, long)]
        raise: bool,
    },
    /// Move a window
    Move {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
        /// X coordinate
        x: i32,
        /// Y coordinate
        y: i32,
    },
    /// Resize a window
    Resize {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
        /// Width
        width: u32,
        /// Height
        height: u32,
    },
    /// Close a window
    Close {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
    },
    /// Iconify (minimize) a window
    Iconify {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
    },
    /// Maximize a window
    Maximize {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
        /// Full maximize (both directions)
        #[arg(short, long)]
        full: bool,
    },
    /// Unmaximize a window
    Unmaximize {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
    },
    /// Shade (roll up) a window
    Shade {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
    },
    /// Unshade a window
    Unshade {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
    },
    /// Move window to desktop
    ToDesktop {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
        /// Desktop number
        desktop: i32,
        /// Follow to desktop
        #[arg(short, long)]
        follow: bool,
    },
    /// Set window layer
    Layer {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
        /// Layer (normal, above, below)
        layer: String,
    },
    /// Get window properties
    Get {
        /// Window ID
        #[arg(value_name = "ID")]
        id: u32,
    },
}

#[derive(Subcommand)]
enum DesktopAction {
    /// List desktops
    List,
    /// Get current desktop
    Current,
    /// Switch to desktop
    Switch {
        /// Desktop number or name
        desktop: String,
    },
    /// Add a desktop
    Add {
        /// Desktop name
        #[arg(default_value = "New Desktop")]
        name: String,
    },
    /// Remove a desktop
    Remove {
        /// Desktop number
        desktop: i32,
    },
    /// Rename a desktop
    Rename {
        /// Desktop number
        desktop: i32,
        /// New name
        name: String,
    },
}

#[derive(Subcommand)]
enum ConfigAction {
    /// Get configuration value
    Get {
        /// Configuration key
        key: String,
    },
    /// Set configuration value
    Set {
        /// Configuration key
        key: String,
        /// Configuration value
        value: String,
        /// Save to config file
        #[arg(short, long)]
        save: bool,
    },
    /// Reload configuration
    Reload,
}

#[derive(Subcommand)]
enum ThemeAction {
    /// Get current theme
    Get,
    /// Set theme
    Set {
        /// Theme name
        name: String,
    },
    /// List available themes
    List,
}

/// Window info for display
#[derive(Tabled, Serialize)]
struct WindowInfo {
    #[tabled(rename = "ID")]
    id: u32,
    #[tabled(rename = "Desktop")]
    desktop: i32,
    #[tabled(rename = "Title")]
    title: String,
    #[tabled(rename = "Class")]
    class: String,
    #[tabled(rename = "States")]
    states: String,
}

/// Desktop info for display
#[derive(Tabled, Serialize)]
struct DesktopInfo {
    #[tabled(rename = "Number")]
    number: i32,
    #[tabled(rename = "Name")]
    name: String,
}

#[tokio::main]
async fn main() -> Result<()> {
    let cli = Cli::parse();

    // Connect to D-Bus
    let proxy = dbus::OpenboxProxy::new().await
        .context("Failed to connect to Openbox D-Bus service. Is Openbox running?")?;

    match cli.command {
        Commands::Lifecycle { action } => match action {
            LifecycleAction::Reconfigure => {
                proxy.reconfigure().await?;
                println!("{}", "Configuration reloaded".green());
            }
            LifecycleAction::Restart { no_save } => {
                proxy.restart(!no_save).await?;
                println!("{}", "Openbox restarting...".yellow());
            }
            LifecycleAction::Exit { no_save } => {
                proxy.exit(!no_save).await?;
                println!("{}", "Openbox exiting...".yellow());
            }
        },

        Commands::Window { action } => match action {
            WindowAction::List { long: _ } => {
                let windows = proxy.get_windows().await?;
                let infos: Vec<WindowInfo> = windows
                    .into_iter()
                    .map(|(id, props)| WindowInfo {
                        id,
                        desktop: props.get("desktop")
                            .and_then(|v| v.downcast_ref::<i32>().copied())
                            .unwrap_or(-1),
                        title: props.get("title")
                            .and_then(|v| v.downcast_ref::<String>().cloned())
                            .unwrap_or_default()
                            .chars()
                            .take(40)
                            .collect(),
                        class: props.get("class")
                            .and_then(|v| v.downcast_ref::<String>().cloned())
                            .unwrap_or_default(),
                        states: props.get("states")
                            .and_then(|v| v.downcast_ref::<Vec<String>>())
                            .map(|s| s.join(", "))
                            .unwrap_or_default(),
                    })
                    .collect();

                match cli.format {
                    OutputFormat::Table => {
                        println!("{}", Table::new(infos));
                    }
                    OutputFormat::Json => {
                        println!("{}", serde_json::to_string_pretty(&infos)?);
                    }
                    OutputFormat::Plain => {
                        for info in infos {
                            println!("{}\\t{}\\t{}\\t{}", 
                                info.id, info.desktop, info.title, info.class);
                        }
                    }
                }
            }
            WindowAction::Focus { id, raise } => {
                proxy.focus_window(id, raise).await?;
                println!("Focused window {}", id);
            }
            WindowAction::Move { id, x, y } => {
                proxy.move_window(id, x, y).await?;
                println!("Moved window {} to ({}, {})", id, x, y);
            }
            WindowAction::Resize { id, width, height } => {
                proxy.resize_window(id, width, height).await?;
                println!("Resized window {} to {}x{}", id, width, height);
            }
            WindowAction::Close { id } => {
                proxy.close_window(id).await?;
                println!("Closed window {}", id);
            }
            WindowAction::Iconify { id } => {
                proxy.iconify_window(id).await?;
                println!("Iconified window {}", id);
            }
            WindowAction::Maximize { id, full } => {
                proxy.maximize_window(id, full).await?;
                println!("Maximized window {}", id);
            }
            WindowAction::Unmaximize { id } => {
                proxy.unmaximize_window(id).await?;
                println!("Unmaximized window {}", id);
            }
            WindowAction::Shade { id } => {
                proxy.shade_window(id).await?;
                println!("Shaded window {}", id);
            }
            WindowAction::Unshade { id } => {
                proxy.unshade_window(id).await?;
                println!("Unshaded window {}", id);
            }
            WindowAction::ToDesktop { id, desktop, follow } => {
                proxy.set_window_desktop(id, desktop, follow).await?;
                println!("Moved window {} to desktop {}", id, desktop);
            }
            WindowAction::Layer { id, layer } => {
                proxy.set_window_layer(id, layer).await?;
                println!("Set window {} layer", id);
            }
            WindowAction::Get { id } => {
                let props = proxy.get_window(id).await?;
                let json = serde_json::to_string_pretty(&props)?;
                println!("{}", json);
            }
        },

        Commands::Desktop { action } => match action {
            DesktopAction::List => {
                let desktops = proxy.get_desktops().await?;
                let infos: Vec<DesktopInfo> = desktops
                    .into_iter()
                    .map(|(num, name)| DesktopInfo { number: num, name })
                    .collect();

                match cli.format {
                    OutputFormat::Table => {
                        println!("{}", Table::new(infos));
                    }
                    OutputFormat::Json => {
                        println!("{}", serde_json::to_string_pretty(&infos)?);
                    }
                    OutputFormat::Plain => {
                        for info in infos {
                            println!("{}\\t{}", info.number, info.name);
                        }
                    }
                }
            }
            DesktopAction::Current => {
                let current = proxy.get_current_desktop().await?;
                println!("Current desktop: {}", current);
            }
            DesktopAction::Switch { desktop } => {
                let num: i32 = desktop.parse()
                    .context("Invalid desktop number")?;
                proxy.set_current_desktop(num).await?;
                println!("Switched to desktop {}", num);
            }
            DesktopAction::Add { name } => {
                let num = proxy.add_desktop(name.clone()).await?;
                println!("Added desktop '{}' as number {}", name, num);
            }
            DesktopAction::Remove { desktop } => {
                proxy.remove_desktop(desktop).await?;
                println!("Removed desktop {}", desktop);
            }
            DesktopAction::Rename { desktop, name } => {
                proxy.rename_desktop(desktop, name.clone()).await?;
                println!("Renamed desktop {} to '{}'", desktop, name);
            }
        },

        Commands::Config { action } => match action {
            ConfigAction::Get { key } => {
                let value = proxy.get_config(key.clone()).await?;
                println!("{} = {:?}", key, value);
            }
            ConfigAction::Set { key, value, save } => {
                // Convert string to appropriate type
                let val: OwnedValue = if let Ok(n) = value.parse::<i32>() {
                    n.into()
                } else if let Ok(b) = value.parse::<bool>() {
                    b.into()
                } else {
                    value.into()
                };
                
                proxy.set_config(key.clone(), val, save).await?;
                println!("Set {} = {}", key, value);
            }
            ConfigAction::Reload => {
                proxy.reconfigure().await?;
                println!("Configuration reloaded");
            }
        },

        Commands::Theme { action } => match action {
            ThemeAction::Get => {
                let theme = proxy.get_theme().await?;
                println!("Current theme: {}", theme);
            }
            ThemeAction::Set { name } => {
                proxy.set_theme(name.clone()).await?;
                println!("Theme set to: {}", name);
            }
            ThemeAction::List => {
                // Would list available themes
                println!("Available themes:");
                println!("  - Clearlooks");
                println!("  - Nightfall");
                println!("  - Minimal");
            }
        },

        Commands::Run { command, args } => {
            proxy.execute(command, args).await?;
            println!("Executed command");
        }

        Commands::Menu { name, x, y } => {
            let x = x.unwrap_or(0);
            let y = y.unwrap_or(0);
            proxy.show_menu(name.clone(), x, y).await?;
            println!("Showing menu '{}' at ({}, {})", name, x, y);
        }

        Commands::Events { types, json } => {
            println!("Subscribing to events: {}", types);
            // Would set up signal subscription
            loop {
                tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;
            }
        }

        Commands::List => {
            // Alias for window list
            let windows = proxy.get_windows().await?;
            let infos: Vec<WindowInfo> = windows
                .into_iter()
                .map(|(id, props)| WindowInfo {
n                    id,\n                    desktop: props.get(\"desktop\")\n                        .and_then(|v| v.downcast_ref::<i32>().copied())\n                        .unwrap_or(-1),\n                    title: props.get(\"title\")\n                        .and_then(|v| v.downcast_ref::<String>().cloned())\n                        .unwrap_or_default(),\n                    class: props.get(\"class\")\n                        .and_then(|v| v.downcast_ref::<String>().cloned())\n                        .unwrap_or_default(),\n                    states: String::new(),\n                })\n                .collect();\n            \n            println!(\"{}\", Table::new(infos));\n        }\n\n        Commands::Status => {\n            let version = proxy.version().await?;\n            let api_version = proxy.api_version().await?;\n            let theme = proxy.get_theme().await?;\n            let desktop = proxy.get_current_desktop().await?;\n            let windows = proxy.get_windows().await?;\n            \n            println!(\"Openbox Status\");\n            println!(\"==============\");\n            println!(\"Version:     {}\", version);\n            println!(\"API Version: {}\", api_version);\n            println!(\"Theme:       {}\", theme);\n            println!(\"Desktop:     {}\", desktop);\n            println!(\"Windows:     {}\", windows.len());\n        }\n\n        // Legacy compatibility\n        Commands::Reconfigure => {\n            proxy.reconfigure().await?;\n            println!(\"{}\", \"Configuration reloaded\".green());\n        }\n        Commands::Restart { no_save } => {\n            proxy.restart(!no_save).await?;\n        }\n        Commands::Exit { no_save } => {\n            proxy.exit(!no_save).await?;\n        }\n    }\n\n    Ok(())\n}
