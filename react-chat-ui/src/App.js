import React, { useState, useEffect } from 'react';
import './App.css';
import useWebSocket from './hooks/useWebSocket';
import StatusIndicator from './components/StatusIndicator';
import MessageList from './components/MessageList';
import MessageInput from './components/MessageInput';

// Configuration for WebSocket URL
const WEBSOCKET_URL = 'ws://127.0.0.1:8080'; // Or use environment variable

function App() {
  const { messages, readyState, error, sendMessage } = useWebSocket(WEBSOCKET_URL);
  const [myNickname, setMyNickname] = useState(localStorage.getItem('chatNickname') || '');
  const [tempNickname, setTempNickname] = useState('');

  // Effect to save nickname to localStorage whenever it changes
  useEffect(() => {
    if (myNickname) {
      localStorage.setItem('chatNickname', myNickname);
    }
  }, [myNickname]);

  const handleSetNickname = () => {
    if (tempNickname.trim()) {
      setMyNickname(tempNickname.trim());
      const nicknameMessage = {
        type: "client_set_nickname",
        payload: { nickname: tempNickname.trim() }
      };
      sendMessage(nicknameMessage);
      // tempNickname input will be hidden by the conditional rendering on myNickname
    }
  };

  const handleSendMessage = (text) => {
    if (text && myNickname) { // Ensure nickname is set before sending messages
      const messageObject = {
        type: "client_send_message",
        payload: {
          // Include nickname primarily for client-side consistency or direct display if needed.
          // Server will use its own session-bound nickname for broadcasts.
          nickname: myNickname,
          text: text,
        }
      };
      sendMessage(messageObject);
    } else if (!myNickname) {
      // Optionally, alert the user or handle this case more gracefully
      console.warn("Nickname not set. Please set a nickname to send messages.");
      alert("Please set your nickname before sending messages.");
    }
  };

  if (!myNickname) {
    return (
      <div className="App App-nickname-prompt">
        <header className="App-header">
          <h1>Welcome to Chat</h1>
        </header>
        <main className="App-main">
          <h2>Set Your Nickname</h2>
          <div className="nickname-input-container">
            <input
              type="text"
              placeholder="Enter your nickname"
              value={tempNickname}
              onChange={(e) => setTempNickname(e.target.value)}
              onKeyPress={(e) => e.key === 'Enter' && handleSetNickname()}
              className="nickname-input"
            />
            <button onClick={handleSetNickname} className="nickname-button">
              Set Nickname
            </button>
          </div>
          {readyState !== WebSocket.OPEN && (
            <div style={{ marginTop: '10px', color: 'orange' }}>
              Connecting to server... Please wait.
            </div>
          )}
           {error && <div style={{ color: 'red', padding: '10px' }}>Error: {error.message} (Try refreshing)</div>}
        </main>
      </div>
    );
  }

  return (
    <div className="App">
      <header className="App-header">
        <h1>React WebSocket Chat</h1>
        <p className="App-my-nickname">My Nickname: <strong>{myNickname}</strong></p>
      </header>
      <main className="App-main">
        <StatusIndicator readyState={readyState} />
        {error && <div style={{ color: 'red', padding: '10px' }}>Error: {error.message}</div>}
        <MessageList messages={messages} myNickname={myNickname} />
        <MessageInput onSendMessage={handleSendMessage} disabled={!myNickname} />
      </main>
    </div>
  );
}

export default App;
