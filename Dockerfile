# Use an official Ubuntu base image
FROM ubuntu:20.04

# Set environment variables to non-interactive to avoid prompts during build
ENV DEBIAN_FRONTEND=noninteractive

# Update the package list and install dependencies
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    cmake \
    libsdl2-dev \
    libboost-all-dev \
    libssl-dev \
    libsndfile-dev \
    nginx \
    git \
    openssl && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Set up the working directory
WORKDIR /app

# Copy the source code into the container
COPY ws-app /app/ws-app

# Build the application
WORKDIR /app/ws-app
RUN mkdir build && cd build && cmake .. && make

# Generate a self-signed certificate in the build directory
RUN openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
    -keyout /app/ws-app/build/server.key \
    -out /app/ws-app/build/server.crt \
    -subj "/C=US/ST=State/L=City/O=Organization/OU=Unit/CN=localhost"

# Configure nginx to serve from the build directory
RUN rm /etc/nginx/sites-enabled/default
COPY nginx.conf /etc/nginx/conf.d/default.conf

# Expose port 8081 for nginx and port 8080 for the ws_undertone application
EXPOSE 8081 8080

# Specify the command to start both the application and the server
CMD service nginx start && ./build/ws_undertone
