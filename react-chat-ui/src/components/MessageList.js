import React from 'react';

const MessageList = ({ messages, myNickname }) => { // Added myNickname prop
  if (!messages || messages.length === 0) {
    return <div className="message-list-empty">No messages yet.</div>;
  }

  const renderPayload = (msgType, payload) => {
    if (!payload) return 'No payload';

    const timestamp = payload.timestamp ? `(${new Date(payload.timestamp).toLocaleTimeString()})` : '';

    switch (msgType) {
      case 'server_broadcast_message':
        // Check if nickname exists, otherwise fall back to user_id
        const displayName = payload.nickname || payload.user_id || 'Unknown User';
        return (
          <>
            <strong>{displayName}{payload.nickname && payload.user_id ? ` (${payload.user_id})` : ''}: </strong>
            {payload.text} <span className="timestamp">{timestamp}</span>
          </>
        );
      case 'server_client_connected':
        return (
          <em>
            User {payload.nickname || payload.user_id || 'Someone'} has connected. {payload.user_id && payload.nickname ? `(ID: ${payload.user_id})` : ''} <span className="timestamp">{timestamp}</span>
          </em>
        );
      case 'server_client_disconnected':
        return (
          <em>
            User {payload.nickname || payload.user_id || 'Someone'} has disconnected. {payload.user_id && payload.nickname ? `(ID: ${payload.user_id})` : ''} <span className="timestamp">{timestamp}</span>
          </em>
        );
      case 'raw_data': // Example for system messages from useWebSocket hook (if any)
         return `System: ${payload.text || JSON.stringify(payload)} ${timestamp}`;
      default:
        return JSON.stringify(payload);
    }
  };

  return (
    <ul className="message-list">
      {messages.map((msg, index) => {
        const isMyMessage = msg.payload && msg.payload.nickname === myNickname && msg.type === 'server_broadcast_message';
        // Basic styling for my messages, can be expanded in CSS
        const messageStyle = isMyMessage
          ? { backgroundColor: '#e1ffc7', alignSelf: 'flex-end', textAlign: 'right' }
          : {};

        return (
          <li key={index} className={`message-item ${isMyMessage ? 'my-message' : ''}`} style={messageStyle}>
            {/* Optional: Display message type for debugging or specific UI needs */}
            {/* <div className="message-type">Type: {msg.type}</div> */}
            <div className="message-content">
              {renderPayload(msg.type, msg.payload)}
            </div>
          </li>
        );
      })}
    </ul>
  );
};

export default MessageList;
