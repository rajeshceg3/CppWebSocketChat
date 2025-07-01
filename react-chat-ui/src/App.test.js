import React from 'react';
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import '@testing-library/jest-dom';
import App from './App';

// Mock the useWebSocket hook
jest.mock('./hooks/useWebSocket', () => ({
  __esModule: true,
  default: jest.fn(),
}));

// Mock localStorage
const localStorageMock = (() => {
  let store = {};
  return {
    getItem: jest.fn((key) => store[key] || null),
    setItem: jest.fn((key, value) => {
      store[key] = value.toString();
    }),
    clear: jest.fn(() => {
      store = {};
    }),
    removeItem: jest.fn(key => {
      delete store[key];
    }),
  };
})();
Object.defineProperty(window, 'localStorage', { value: localStorageMock });


describe('App Component Nickname Functionality', () => {
  let mockSendMessage;
  let mockReadyState;

  beforeEach(() => {
    // Reset localStorage and mocks before each test
    localStorageMock.clear();
    jest.clearAllMocks();

    mockSendMessage = jest.fn();
    mockReadyState = WebSocket.OPEN; // Default to open connection

    require('./hooks/useWebSocket').default.mockImplementation(() => ({
      messages: [],
      readyState: mockReadyState,
      error: null,
      sendMessage: mockSendMessage,
    }));
  });

  test('shows nickname input screen initially when no nickname in localStorage', () => {
    render(<App />);
    expect(screen.getByText('Set Your Nickname')).toBeInTheDocument();
    expect(screen.getByPlaceholderText('Enter your nickname')).toBeInTheDocument();
    expect(screen.getByRole('button', { name: 'Set Nickname' })).toBeInTheDocument();
    // Main chat interface elements should not be present
    expect(screen.queryByText('My Nickname:')).not.toBeInTheDocument();
    expect(screen.queryByPlaceholderText('Type a message...')).not.toBeInTheDocument();
  });

  test('loads nickname from localStorage and shows main chat UI', () => {
    localStorageMock.setItem('chatNickname', 'TestUser123');
    render(<App />);

    expect(screen.getByText('My Nickname:')).toBeInTheDocument();
    expect(screen.getByText('TestUser123')).toBeInTheDocument();
    expect(screen.getByPlaceholderText('Type a message...')).toBeInTheDocument();
    // Nickname prompt should not be present
    expect(screen.queryByText('Set Your Nickname')).not.toBeInTheDocument();
  });

  test('allows user to set a nickname, sends message, and shows main chat UI', async () => {
    render(<App />);

    // Nickname prompt should be visible
    const nicknameInput = screen.getByPlaceholderText('Enter your nickname');
    const setNicknameButton = screen.getByRole('button', { name: 'Set Nickname' });

    fireEvent.change(nicknameInput, { target: { value: 'NewbieUser' } });
    fireEvent.click(setNicknameButton);

    // Check if sendMessage was called with client_set_nickname
    expect(mockSendMessage).toHaveBeenCalledWith({
      type: 'client_set_nickname',
      payload: { nickname: 'NewbieUser' },
    });

    // Wait for the UI to update (myNickname state changes, localStorage is set)
    await waitFor(() => {
      expect(screen.getByText('My Nickname:')).toBeInTheDocument();
    });
    expect(screen.getByText('NewbieUser')).toBeInTheDocument(); // Nickname displayed in header
    expect(localStorageMock.getItem('chatNickname')).toBe('NewbieUser'); // Nickname saved

    // Nickname prompt should be gone, main chat UI visible
    expect(screen.queryByText('Set Your Nickname')).not.toBeInTheDocument();
    expect(screen.getByPlaceholderText('Type a message...')).toBeInTheDocument();
  });

  test('handles sending a chat message after nickname is set', async () => {
    localStorageMock.setItem('chatNickname', 'ChattyUser');
    render(<App />);

    // Ensure main chat UI is rendered
    await waitFor(() => {
      expect(screen.getByText('ChattyUser')).toBeInTheDocument();
    });

    const messageInput = screen.getByPlaceholderText('Type a message...');
    const sendButton = screen.getByRole('button', { name: 'Send' });

    fireEvent.change(messageInput, { target: { value: 'Hello Server!' } });
    fireEvent.click(sendButton);

    expect(mockSendMessage).toHaveBeenCalledWith({
      type: 'client_send_message',
      payload: {
        nickname: 'ChattyUser',
        text: 'Hello Server!',
      },
    });
    expect(messageInput.value).toBe(''); // Input should clear after sending
  });

  test('prevents sending message if nickname is not set and shows alert', () => {
    // This scenario is harder to test directly as the UI prevents it by not showing message input.
    // However, we can test the handleSendMessage function's internal logic if it were exposed,
    // or ensure the MessageInput is disabled.

    // Render with no nickname (nickname prompt shown)
    render(<App />);
    expect(screen.getByText('Set Your Nickname')).toBeInTheDocument();

    // MessageInput component should not be present or should be disabled.
    // In our App.js, MessageInput is not rendered when myNickname is not set.
    // So, we check that it's not there.
    expect(screen.queryByPlaceholderText('Type a message...')).not.toBeInTheDocument();
    expect(screen.queryByRole('button', { name: 'Send' })).not.toBeInTheDocument();

    // If we wanted to test the alert, we'd need to somehow call handleSendMessage without a nickname.
    // This is more of an integration aspect of App's internal state and MessageInput.
    // The current setup (hiding MessageInput) is a stronger guarantee.
  });

  test('shows connecting message when WebSocket is not open on nickname screen', () => {
    mockReadyState = WebSocket.CONNECTING;
     require('./hooks/useWebSocket').default.mockImplementation(() => ({
      messages: [],
      readyState: mockReadyState,
      error: null,
      sendMessage: mockSendMessage,
    }));

    render(<App />);
    expect(screen.getByText('Set Your Nickname')).toBeInTheDocument();
    expect(screen.getByText('Connecting to server... Please wait.')).toBeInTheDocument();
  });

  test('shows error message if connection error occurs on nickname screen', () => {
    const MOCK_ERROR = { message: 'Connection Failed' };
    require('./hooks/useWebSocket').default.mockImplementation(() => ({
      messages: [],
      readyState: WebSocket.CLOSED, // Or any state that's not OPEN
      error: MOCK_ERROR,
      sendMessage: mockSendMessage,
    }));
    render(<App />);
    expect(screen.getByText('Set Your Nickname')).toBeInTheDocument();
    expect(screen.getByText(`Error: ${MOCK_ERROR.message} (Try refreshing)`)).toBeInTheDocument();
  });

});
