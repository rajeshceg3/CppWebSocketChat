import React, { useState } from 'react';

const MessageInput = ({ onSendMessage }) => {
  const [inputValue, setInputValue] = useState('');

  const handleSubmit = (e) => {
    e.preventDefault();
    if (inputValue.trim()) {
      onSendMessage(inputValue.trim());
      setInputValue(''); // Clear input after sending
    }
  };

  return (
    <form onSubmit={handleSubmit} style={{ display: 'flex', padding: '10px' }}>
      <input
        type="text"
        value={inputValue}
        onChange={(e) => setInputValue(e.target.value)}
        placeholder="Type a message..."
        style={{ flexGrow: 1, padding: '8px', marginRight: '8px', border: '1px solid #ccc', borderRadius: '4px' }}
      />
      <button type="submit" style={{ padding: '8px 15px', border: 'none', backgroundColor: '#007bff', color: 'white', borderRadius: '4px', cursor: 'pointer' }}>
        Send
      </button>
    </form>
  );
};

export default MessageInput;
