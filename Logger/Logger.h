//SEP400 - Assignment 1: Embedded Debug Logging
//Logger.h - Contains all functions and enums needed for the Logger
//Created: Abhi Patel -> apatel477@myseneca.ca
//         Neel Mahimkar -> nmahimkar@myseneca.ca
//Date: Mar 26, 2023.

#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    DEBUG,
    WARNING,
    ERROR,
    CRITICAL
} LOG_LEVEL;

int InitializeLog(); // Initialize the logger
void SetLogLevel(LOG_LEVEL level);  // Set the log level
void Log(LOG_LEVEL level, const char *file, const char *func, int line, const char *message);// Writes a log message
void ExitLog();// Clean up the logger

#endif /* LOGGER_H */
//  **Assignment 1**