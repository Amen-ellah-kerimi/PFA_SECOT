use prettytable::{Cell, Row, Table};
use serde::Serialize;
use std::fmt;

/// Create a table from a vector of serializable items
pub fn create_table<T: Serialize>(items: &[T], headers: &[&str]) -> Result<Table, String> {
    let mut table = Table::new();
    
    // Add headers
    let header_cells: Vec<Cell> = headers.iter().map(|h| Cell::new(h)).collect();
    table.add_row(Row::new(header_cells));
    
    // Add rows
    for item in items {
        let json = serde_json::to_value(item).map_err(|e| e.to_string())?;
        
        if let serde_json::Value::Object(map) = json {
            let cells: Vec<Cell> = headers
                .iter()
                .map(|&header| {
                    let value = map.get(header).cloned().unwrap_or(serde_json::Value::Null);
                    Cell::new(&value.to_string())
                })
                .collect();
            
            table.add_row(Row::new(cells));
        }
    }
    
    Ok(table)
}

/// Format a table with a title
pub struct FormattedTable {
    pub title: String,
    pub table: Table,
}

impl FormattedTable {
    pub fn new(title: &str, table: Table) -> Self {
        Self {
            title: title.to_string(),
            table,
        }
    }
}

impl fmt::Display for FormattedTable {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(f, "\n{}", self.title)?;
        write!(f, "{}", self.table)
    }
}
