import React from 'react';

const StatusIndicator = ({ readyState }) => {
  let statusText = 'Unknown';
  let statusColor = 'grey';

  switch (readyState) {
    case WebSocket.CONNECTING:
      statusText = 'Connecting...';
      statusColor = 'orange';
      break;
    case WebSocket.OPEN:
      statusText = 'Connected';
      statusColor = 'green';
      break;
    case WebSocket.CLOSING:
      statusText = 'Closing...';
      statusColor = 'orange';
      break;
    case WebSocket.CLOSED:
      statusText = 'Disconnected';
      statusColor = 'red';
      break;
    default:
      statusText = 'Unknown State';
      statusColor = 'grey';
  }

  return (
    <div style={{ padding: '10px', borderBottom: '1px solid #ccc' }}>
      Status: <span style={{ color: statusColor, fontWeight: 'bold' }}>{statusText}</span>
    </div>
  );
};

export default StatusIndicator;
