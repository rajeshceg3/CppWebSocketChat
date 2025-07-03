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
    <form onSubmit={handleSubmit} className="MessageInput-container">
      <input
        type="text"
        value={inputValue}
        onChange={(e) => setInputValue(e.target.value)}
        placeholder={disabled ? "Set your nickname to chat" : "Type a message..."}
        disabled={disabled} // Apply disabled attribute
        className="MessageInput-input"
      />
      <button
        type="submit"
        disabled={disabled} // Apply disabled attribute
        className="MessageInput-button"
      >
        Send
      </button>
    </form>
  );
};

export default MessageInput;
