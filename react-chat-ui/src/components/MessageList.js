import React from 'react';

const MessageList = ({ messages }) => {
  if (!messages || messages.length === 0) {
    return <div style={{ padding: '10px', fontStyle: 'italic' }}>No messages yet.</div>;
  }

  const renderPayload = (payload) => {
    if (!payload) return 'No payload';
    // Customize this based on expected payload structures for different message types
    if (payload.user_id && payload.text) {
      return `User ${payload.user_id}: ${payload.text} (${new Date(payload.timestamp).toLocaleTimeString()})`;
    }
    if (payload.user_id && payload.message) { // For connected/disconnected messages
        return `${payload.message} User: ${payload.user_id} (${new Date(payload.timestamp).toLocaleTimeString()})`;
    }
    if (payload.text && payload.user_id === 'System') { // For raw_data or other system messages from hook
        return `System: ${payload.text} (${new Date(payload.timestamp).toLocaleTimeString()})`;
    }
    return JSON.stringify(payload);
  };

  return (
    <ul style={{ listStyleType: 'none', padding: '10px', maxHeight: '400px', overflowY: 'auto', borderBottom: '1px solid #ccc' }}>
      {messages.map((msg, index) => (
        <li key={index} style={{ marginBottom: '8px', padding: '5px', border: '1px solid #eee', borderRadius: '4px' }}>
          <div style={{ fontWeight: 'bold', color: msg.type === 'server_broadcast_message' ? 'blue' : '#555' }}>
            Type: {msg.type}
          </div>
          <div style={{ fontSize: '0.9em', color: '#333' }}>
            {renderPayload(msg.payload)}
          </div>
        </li>
      ))}
    </ul>
  );
};

export default MessageList;
