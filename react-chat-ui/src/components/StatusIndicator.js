import React from 'react';

const StatusIndicator = ({ readyState }) => {
  let statusText = 'Unknown';
  let statusClassName = 'status-unknown';

  switch (readyState) {
    case WebSocket.CONNECTING:
      statusText = 'Connecting...';
      statusClassName = 'status-connecting';
      break;
    case WebSocket.OPEN:
      statusText = 'Connected';
      statusClassName = 'status-connected';
      break;
    case WebSocket.CLOSING:
      statusText = 'Closing...';
      statusClassName = 'status-closing';
      break;
    case WebSocket.CLOSED:
      statusText = 'Disconnected';
      statusClassName = 'status-disconnected';
      break;
    default:
      statusText = 'Unknown State';
      statusClassName = 'status-unknown';
  }

  return (
    <div className="StatusIndicator-container">
      Status: <span className={`StatusIndicator-text ${statusClassName}`}>{statusText}</span>
    </div>
  );
};

export default StatusIndicator;
