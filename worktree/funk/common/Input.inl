bool Input::IsKeyDownInState( int iState, unsigned int key ) const
{
	return ( m_arrayKeyStates[ iState ][ key ] ) != 0;
}

bool Input::IsKeyDown( const char* key ) const
{
	unsigned int rawKey = m_mapStringKey.find(key)->second;
	return IsKeyDownInState( m_iCurrentState^1, rawKey );
}

bool Input::DidKeyJustGoDown( const char* key ) const
{
	unsigned int rawKey = m_mapStringKey.find(key)->second;
	return IsKeyDownInState( m_iCurrentState^1, rawKey ) && !IsKeyDownInState( m_iCurrentState, rawKey );
}

bool Input::DidKeyJustGoUp( const char* key ) const
{
	unsigned int rawKey = m_mapStringKey.find(key)->second;
	return !IsKeyDownInState( m_iCurrentState^1, rawKey )  && IsKeyDownInState( m_iCurrentState, rawKey );
}

bool Input::IsMouseDown( int btn ) const
{
	return IsMouseDownInState(m_iCurrentState^1, btn);
}

bool Input::DidMouseJustGoDown( int btn ) const
{
	return IsMouseDownInState(m_iCurrentState^1, btn) && !IsMouseDownInState(m_iCurrentState, btn);
}

bool Input::DidMouseJustGoUp( int btn ) const
{
	return !IsMouseDownInState(m_iCurrentState^1, btn) && IsMouseDownInState(m_iCurrentState, btn);
}

bool Input::IsMouseDownInState( int iState, int mouseKey ) const
{
	return (m_mouseState[iState] & SDL_BUTTON(mouseKey)) != 0;
}

bool Input::DidMouseWheelHit( int btn ) const
{
	return m_mouseWheelHit[btn];
}
