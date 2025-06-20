// udp-websocket-bridge.js
const dgram = require('dgram');
const WebSocket = require('ws');
const http = require('http');

// Create UDP socket
const udpServer = dgram.createSocket('udp4');
const UDP_PORT = 10245;

// Create HTTP server and WebSocket server
const server = http.createServer();
const wss = new WebSocket.Server({ server });

let clients = new Set();

// Handle new WebSocket connections
wss.on('connection', (ws) => {
    clients.add(ws);
    console.log('WebSocket client connected.');

    ws.on('close', () => {
        clients.delete(ws);
        console.log('WebSocket client disconnected.');
    });
});

// Handle incoming UDP messages
udpServer.on('message', (msg, rinfo) => {
    const message = msg.toString().trim();
    console.log(`Received valid UDP: ${message} from ${rinfo.address}`);

    // Send to all connected WebSocket clients
    for (const client of clients) {
        if (client.readyState === WebSocket.OPEN) {
            client.send(message);
        }
    }
});

// Start servers
udpServer.bind(UDP_PORT, () => {
    console.log(`UDP server listening on port ${UDP_PORT}`);
});

const HTTP_PORT = 8080;
server.listen(HTTP_PORT, () => {
    console.log(`WebSocket server running on http://localhost:${HTTP_PORT}`);
});
