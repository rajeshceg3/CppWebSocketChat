import React, { useState } from 'react';

const MessageInput = ({ onSendMessage, disabled }) => { // Added disabled prop
  const [inputValue, setInputValue] = useState('');

  const handleSubmit = (e) => {
    e.preventDefault();
    if (inputValue.trim() && !disabled) { // Check disabled state
      onSendMessage(inputValue.trim());
      setInputValue(''); // Clear input after sending
    }
  };

  return (
    // Added className for potential global styling from App.css
    <form onSubmit={handleSubmit} className="MessageInput-container" style={{ display: 'flex', padding: '10px' }}>
      <input
        type="text"
        value={inputValue}
        onChange={(e) => setInputValue(e.target.value)}
        placeholder={disabled ? "Set your nickname to chat" : "Type a message..."}
        disabled={disabled} // Apply disabled attribute
        style={{ flexGrow: 1, padding: '8px', marginRight: '8px', border: '1px solid #ccc', borderRadius: '4px' }}
      />
      <button
        type="submit"
        disabled={disabled} // Apply disabled attribute
        style={{ padding: '8px 15px', border: 'none', backgroundColor: disabled ? '#ccc' : '#007bff', color: 'white', borderRadius: '4px', cursor: disabled ? 'not-allowed' : 'pointer' }}
      >
        Send
      </button>
    </form>
  );
};

export default MessageInput;
