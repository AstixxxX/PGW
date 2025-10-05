# Packet Gateway (PGW) System

## Overview
The Packet Gateway (PGW) is a network session management system that handles communication between mobile devices and network services. It consists of three core interconnected components:
1. **PGW Client** - Initiates sessions via UDP requests  
2. **PGW Server** - Processes and manages sessions  
3. **PGW Web Panel** - Provides monitoring and control interface  

## System Components

### 1. PGW Client (Initiates device sessions by transmitting IMSI to the server)  
**Functions**:  
- Send IMSI payloads via UDP protocol  
- Await session status responses from server  
- Log transaction results  
- **Configuration**: `client_config.json`  

### 2. PGW Server (Creates, maintains, and terminates mobile device sessions)
**Core Components**:  
1. **UDP Server**  
  - Default Port: `10000`  
  - Functions:  
     - Receives IMSI payloads from clients  
     - Generates UDP response packets  
     - Logs all incoming requests  

2. **HTTP Server**
  - Default Port: `8080`  
 - API Endpoints:  
    - `POST /api/check_subscriber` - Verify session status by IMSI  
     - `POST /api/stop` - Gracefully terminate PGW Server  

3. **Session Controller**  
 - Workflow:  
     - Receives IMSI from UDP Server  
     - Makes access control decisions  
   - **Configuration**: `server_config.json`  

### 3. PGW Web Interface (Remote administration and monitoring of PGW Server)  
**Functions**:  
 - Real-time session monitoring  
 - System configuration management  
 - **Configuration**: Requires PGW Server URL
