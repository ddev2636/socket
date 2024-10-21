# Define variables
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11
SERVER = server/server
CLIENT = client/client
SERVER_SRC = server/server.cpp
CLIENT_SRC = client/client.cpp

# Default target
all: $(SERVER) $(CLIENT)

# Compile server
$(SERVER): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $(SERVER) $(SERVER_SRC)

# Compile client
$(CLIENT): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o $(CLIENT) $(CLIENT_SRC)

# Run both server and client
run: $(SERVER) $(CLIENT)
	@echo "Starting server in background..."
	./$(SERVER) &   # Start server in background
	@sleep 2        # Give the server a second to start
	@echo "Starting client..."
	./$(CLIENT)     # Run client

# Clean up generated files
clean:
	rm -f $(SERVER) $(CLIENT)

.PHONY: all run clean
