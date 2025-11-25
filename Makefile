# Device Tracker Makefile
# Hỗ trợ compile trên Windows (MinGW) và Linux

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2

# Libraries
# Windows: cần link với ws2_32, iphlpapi, curl
# Linux: chỉ cần curl
ifeq ($(OS),Windows_NT)
    LIBS = -lcurl -lws2_32 -liphlpapi
    TARGET = device_tracker.exe
    RM = del /Q
else
    LIBS = -lcurl
    TARGET = device_tracker
    RM = rm -f
endif

# Source files
SRCS = main.c
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJS)
	@echo Linking $(TARGET)...
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)
	@echo Build complete!

# Compile source files
%.o: %.c config.h
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	@echo Cleaning...
ifeq ($(OS),Windows_NT)
	if exist *.o $(RM) *.o
	if exist $(TARGET) $(RM) $(TARGET)
else
	$(RM) $(OBJS) $(TARGET)
endif
	@echo Clean complete!

# Run the program
run: $(TARGET)
	./$(TARGET)

# Install (copy to a system directory - requires admin)
install: $(TARGET)
ifeq ($(OS),Windows_NT)
	@echo Please manually copy $(TARGET) to your desired location
else
	sudo cp $(TARGET) /usr/local/bin/
	@echo Installed to /usr/local/bin/$(TARGET)
endif

# Uninstall
uninstall:
ifeq ($(OS),Windows_NT)
	@echo Please manually remove the executable
else
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo Uninstalled from /usr/local/bin/
endif

# Help
help:
	@echo Device Tracker Build System
	@echo.
	@echo Available targets:
	@echo   all       - Build the project (default)
	@echo   clean     - Remove build files
	@echo   run       - Build and run the program
	@echo   install   - Install to system directory
	@echo   uninstall - Remove from system directory
	@echo   help      - Show this help message
	@echo.
	@echo Requirements:
	@echo   - GCC compiler (MinGW on Windows)
	@echo   - libcurl library

.PHONY: all clean run install uninstall help
