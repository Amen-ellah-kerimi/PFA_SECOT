use anyhow::Result;
use console::style;
use serde::Serialize;
use std::fmt::Display;

/// Format output based on the specified format
pub fn format_output<T: Serialize + Display>(data: &T, format: &str) -> Result<String> {
    match format {
        "json" => {
            let json = serde_json::to_string_pretty(data)?;
            Ok(json)
        }
        "table" => {
            // For table format, we rely on the Display implementation
            Ok(format!("{}", data))
        }
        _ => {
            // Default to table format
            Ok(format!("{}", data))
        }
    }
}

/// Print a success message
pub fn print_success(message: &str) {
    println!("{} {}", style("✓").green().bold(), message);
}

/// Print an error message
pub fn print_error(message: &str) {
    eprintln!("{} {}", style("✗").red().bold(), message);
}

/// Print a warning message
pub fn print_warning(message: &str) {
    println!("{} {}", style("!").yellow().bold(), message);
}

/// Print an info message
pub fn print_info(message: &str) {
    println!("{} {}", style("i").blue().bold(), message);
}

/// Print a section header
pub fn print_section(title: &str) {
    println!("\n{}", style(title).bold().underlined());
}

/// Print a command result
pub fn print_command_result(command: &str, result: &str, success: bool) {
    let status = if success {
        style("SUCCESS").green().bold()
    } else {
        style("FAILED").red().bold()
    };
    
    println!("{}: {} - {}", command, status, result);
}
