import React from 'react';
import './App.css';
import useWebSocket from './hooks/useWebSocket';
import StatusIndicator from './components/StatusIndicator';
import MessageList from './components/MessageList';
import MessageInput from './components/MessageInput';

// Configuration for WebSocket URL
const WEBSOCKET_URL = 'ws://127.0.0.1:8080'; // Or use environment variable

function App() {
  const { messages, readyState, error, sendMessage } = useWebSocket(WEBSOCKET_URL);

  const handleSendMessage = (text) => {
    if (text) {
      const messageObject = {
        type: "client_send_message",
        payload: {
          text: text,
        }
      };
      sendMessage(messageObject);
    }
  };

  return (
    <div className="App">
      <header className="App-header">
        <h1>React WebSocket Chat</h1>
      </header
      <main className="App-main">
        <StatusIndicator readyState={readyState} />
        {error && <div style={{ color: 'red', padding: '10px' }}>Error: {error.message}</div>}
        <MessageList messages={messages} />
        <MessageInput onSendMessage={handleSendMessage} />
      </main>
    </div>
  );
}

export default App;
