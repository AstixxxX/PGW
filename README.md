# Packet Gateway (PGW) System

## Overview

PGW (Packet Gateway) is a network session management system consisting of three interconnected components:

1. **PGW Client** - sends UDP requests  
2. **PGW Server** - processes sessions  
3. **Web Panel**  - management interface  

## System Components  

### 1. PGW Client  

**Purpose**: Initiates sessions by sending IMSI to server  

**Functions**:  
   - Sends IMSI via UDP  
   - Waits for session status response   
   - Logs results  

**Configuration**: (client_config.json)

### 2. PGW Server

**Purpose**: Create and update sessions with mobile devices (clients)

**Server Components**:
1. **UDP Server**
   - Port: 10000 (default)
   - Functions:
     - Receives IMSI from clients
     - Generates UDP response packets
     - Logs incoming requests

2. **HTTP Server**
   - Port: 8080 (default)
   - API Endpoints:
     - `POST /api/check_subscriber` - check session by IMSI
     - `POST /api/stop` - stop the PGW server

3. **Session Controller**
   - Workflow:
     1. Receives IMSI from UDP Server
     2. Makes access decision
     3. Forms response for client

**Configuration**: (server_config.json)

### 3. PGW Web Interface

**Purpose**: Remote control of PGW server

**Functions**
 - Session monitoring
 - System configuration

**Configuration** (server url)
