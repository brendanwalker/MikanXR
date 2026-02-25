/**
 * Example usage of the MikanXR TypeScript client
 * This shows how to connect to Mikan, send requests, and handle events
 */

import {
  MikanClient,
  MikanLogLevel,
  InitClientRequest,
  GetSpatialAnchorList,
  CLASS_ID_INIT_CLIENT_REQUEST,
  CLASS_ID_GET_SPATIAL_ANCHOR_LIST
} from '../index';

async function main() {
  // Create and initialize the client
  const client = new MikanClient({
    host: 'localhost',
    port: '8080',
    autoReconnect: false
  });

  // Set up logging
  client.setLogCallback((level: MikanLogLevel, message: string) => {
    console.log(`[${MikanLogLevel[level]}] ${message}`);
  });

  // Initialize
  const initResult = client.initialize(MikanLogLevel.Debug);
  if (initResult !== 0) {
    console.error('Failed to initialize client');
    return;
  }

  // Set up event listeners
  client.events.onConnected((event) => {
    console.log('Connected to Mikan server!', event);
  });

  client.events.onDisconnected((event) => {
    console.log('Disconnected from Mikan server:', event.code, event.reason);
  });

  client.events.onAnchorPoseUpdate((event) => {
    console.log('Anchor pose updated:', event.anchor_id, event.transform);
  });

  client.events.onAnchorNameUpdate((event) => {
    console.log('Anchor name updated:', event.anchor_id, event.anchor_name);
  });

  // Connect to the server
  console.log('Connecting to Mikan server...');
  const connectResult = await client.connect();

  if (connectResult !== 0) {
    console.error('Failed to connect to server');
    return;
  }

  // Send initialization request
  const clientInfo = client.createClientInfo(
    'Electron',
    '1.0.0',
    'MikanXR Electron Client',
    '1.0.0',
    'Desktop'
  );

  const initRequest: InitClientRequest = {
    requestTypeId: CLASS_ID_INIT_CLIENT_REQUEST,
    requestTypeName: 'InitClientRequest',
    requestId: 0,
    clientInfo
  };

  console.log('Sending init request...');
  const initResponse = await client.sendRequest(initRequest);
  console.log('Init response:', initResponse);

  // Request spatial anchor list
  const anchorListRequest: GetSpatialAnchorList = {
    requestTypeId: CLASS_ID_GET_SPATIAL_ANCHOR_LIST,
    requestTypeName: 'GetSpatialAnchorList',
    requestId: 0
  };

  console.log('Requesting spatial anchor list...');
  const anchorListResponse = await client.sendRequest(anchorListRequest);
  console.log('Anchor list response:', anchorListResponse);

  // Keep the connection alive for events
  console.log('Listening for events... (Press Ctrl+C to exit)');

  // In a real app, you would keep this running and handle events
  // For this example, we'll disconnect after 30 seconds
  setTimeout(() => {
    console.log('Disconnecting...');
    client.disconnect();
    client.shutdown();
  }, 30000);
}

// Run the example
main().catch(console.error);
