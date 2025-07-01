import React from 'react';
import { render, screen } from '@testing-library/react';
import '@testing-library/jest-dom';
import MessageList from './MessageList';

describe('MessageList Component', () => {
  const baseTimestamp = new Date().toISOString();
  const mockMyNickname = "MyOwnNick";

  test('renders "No messages yet" when messages array is empty or null', () => {
    render(<MessageList messages={[]} myNickname={mockMyNickname} />);
    expect(screen.getByText('No messages yet.')).toBeInTheDocument();

    render(<MessageList messages={null} myNickname={mockMyNickname} />);
    expect(screen.getByText('No messages yet.')).toBeInTheDocument();
  });

  test('renders server_broadcast_message with nickname and user_id', () => {
    const messages = [
      {
        type: 'server_broadcast_message',
        payload: {
          user_id: 'user123',
          nickname: 'SenderNick',
          text: 'Hello world!',
          timestamp: baseTimestamp
        }
      },
    ];
    render(<MessageList messages={messages} myNickname={mockMyNickname} />);
    expect(screen.getByText((content, element) => {
      return element.tagName.toLowerCase() === 'strong' && content.startsWith('SenderNick');
    })).toBeInTheDocument();
    expect(screen.getByText((content, element) => {
      return element.textContent.includes('(user123):');
    })).toBeInTheDocument();
    expect(screen.getByText((content, element) => {
        return element.textContent.includes('Hello world!');
    })).toBeInTheDocument();
  });

  test('renders server_broadcast_message with only user_id if nickname is missing', () => {
    const messages = [
      {
        type: 'server_broadcast_message',
        payload: {
          user_id: 'user456',
          text: 'No nickname message',
          timestamp: baseTimestamp
        }
      },
    ];
    render(<MessageList messages={messages} myNickname={mockMyNickname} />);
    expect(screen.getByText((content, element) => {
      return element.tagName.toLowerCase() === 'strong' && content.startsWith('user456');
    })).toBeInTheDocument();
     expect(screen.getByText((content, element) => {
        return element.textContent.includes('No nickname message');
    })).toBeInTheDocument();
  });

  test('renders server_broadcast_message from "Unknown User" if nickname and user_id are missing', () => {
    const messages = [
      {
        type: 'server_broadcast_message',
        payload: {
          text: 'Totally anonymous message',
          timestamp: baseTimestamp
        }
      },
    ];
    render(<MessageList messages={messages} myNickname={mockMyNickname} />);
    expect(screen.getByText((content, element) => {
      return element.tagName.toLowerCase() === 'strong' && content.startsWith('Unknown User');
    })).toBeInTheDocument();
  });


  test('renders server_client_connected message with nickname', () => {
    const messages = [
      {
        type: 'server_client_connected',
        payload: {
          user_id: 'user789',
          nickname: 'NewUser',
          message: 'User has connected.',
          timestamp: baseTimestamp
        }
      },
    ];
    render(<MessageList messages={messages} myNickname={mockMyNickname} />);
    expect(screen.getByText(/User NewUser \(ID: user789\) has connected./)).toBeInTheDocument();
  });

  test('renders server_client_connected message with user_id if nickname missing', () => {
    const messages = [
      {
        type: 'server_client_connected',
        payload: {
          user_id: 'user789',
          message: 'User has connected.',
          timestamp: baseTimestamp
        }
      },
    ];
    render(<MessageList messages={messages} myNickname={mockMyNickname} />);
    expect(screen.getByText(/User user789 has connected./)).toBeInTheDocument();
  });


  test('renders server_client_disconnected message with nickname', () => {
    const messages = [
      {
        type: 'server_client_disconnected',
        payload: {
          user_id: 'userABC',
          nickname: 'OldUser',
          message: 'User has disconnected.',
          timestamp: baseTimestamp
        }
      },
    ];
    render(<MessageList messages={messages} myNickname={mockMyNickname} />);
    expect(screen.getByText(/User OldUser \(ID: userABC\) has disconnected./)).toBeInTheDocument();
  });

  test('applies "my-message" class and style to messages sent by myNickname', () => {
    const messages = [
      {
        type: 'server_broadcast_message',
        payload: {
          user_id: 'myUserEntity',
          nickname: mockMyNickname,
          text: 'This is my message!',
          timestamp: baseTimestamp
        }
      },
      {
        type: 'server_broadcast_message',
        payload: {
          user_id: 'otherUserEntity',
          nickname: 'OtherUser',
          text: 'This is other user message!',
          timestamp: baseTimestamp
        }
      },
    ];
    render(<MessageList messages={messages} myNickname={mockMyNickname} />);

    const myMessageItem = screen.getByText((content, element) => element.textContent.includes('This is my message!')).closest('li');
    expect(myMessageItem).toHaveClass('my-message');
    // Check for inline styles (more brittle, but part of the spec)
    // Example: expect(myMessageItem).toHaveStyle('backgroundColor: #e1ffc7');
    // Note: toHaveStyle might require jest-styled-components or similar for complex style checks if not inline.
    // For basic inline styles added directly in JS, it might work.
    // The component applies { backgroundColor: '#e1ffc7', alignSelf: 'flex-end', textAlign: 'right' }

    const otherMessageItem = screen.getByText((content, element) => element.textContent.includes('This is other user message!')).closest('li');
    expect(otherMessageItem).not.toHaveClass('my-message');
  });

  test('renders multiple messages correctly', () => {
    const messages = [
      { type: 'server_client_connected', payload: { user_id: 'u1', nickname: 'Nick1', message: '...', timestamp: baseTimestamp } },
      { type: 'server_broadcast_message', payload: { user_id: 'u2', nickname: 'Nick2', text: 'Text1', timestamp: baseTimestamp } },
      { type: 'server_client_disconnected', payload: { user_id: 'u1', nickname: 'Nick1', message: '...', timestamp: baseTimestamp } },
    ];
    render(<MessageList messages={messages} myNickname={mockMyNickname} />);
    expect(screen.getByText(/User Nick1 \(ID: u1\) has connected./)).toBeInTheDocument();
    expect(screen.getByText((content, el) => el.textContent.includes('Nick2 (u2): Text1'))).toBeInTheDocument();
    expect(screen.getByText(/User Nick1 \(ID: u1\) has disconnected./)).toBeInTheDocument();
  });

  test('renders messages with only timestamp if other fields are missing', () => {
     const messages = [
      {
        type: 'server_broadcast_message',
        payload: {
          timestamp: baseTimestamp
        }
      },
    ];
    render(<MessageList messages={messages} myNickname={mockMyNickname} />);
    // Checks that "Unknown User" is displayed and the timestamp part is there
    expect(screen.getByText((content, el) => el.textContent.startsWith('Unknown User') && el.textContent.includes(new Date(baseTimestamp).toLocaleTimeString()))).toBeInTheDocument();
  });

});
