pub struct AppConfig {
    pub log_directory: String,
    pub capture_directory: String,
    pub dictionary_directory: String,
}

impl Default for AppConfig {
    fn default() -> Self {
        AppConfig {
            log_directory: "logs".to_string(),
            capture_directory: "captures".to_string(),
            dictionary_directory: "dictionaries".to_string(),
        }
    }
}
