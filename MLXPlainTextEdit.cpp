/*
*
* Copyright 2025 prov3ntus
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include "stdafx.h"
#include "MLXPlainTextEdit.h"

void MLXPlainTextEdit::appendColoredText( const QString &text )
{
	// Save the current cursor position
	QTextCursor originalCursor = this->textCursor();

	// Move the cursor to the end of the document
	QTextCursor cursor( this->document() );
	cursor.movePosition( QTextCursor::End );

	setTextCursor( cursor );  // Set the cursor to the end

	// Split text into segments based on color codes.
	QList<QString> segments = text.split( QRegularExpression( "(?=\\^\\d)" ), Qt::SkipEmptyParts );
	for( QString &segment : segments )
	{
		// Add timestamp when a new block starts
		if( cursor.atBlockStart() && !segment.startsWith( '*' ) )
		{
			QString timestamp = QTime::currentTime().toString( "[hh:mm:ss.zzz] |> " );
			QTextCharFormat timeFormat;
			timeFormat.setForeground( Qt::gray );
			cursor.setCharFormat( timeFormat );
			cursor.insertText( timestamp );
			//this->insertPlainText( timestamp );
		}

		handleColorCodes( segment, cursor );
	}

	// Restore the original cursor position (optional)
	setTextCursor( originalCursor );
}

void MLXPlainTextEdit::handleColorCodes( QString &segment, QTextCursor &cursor )
{
	QColor color = Qt::white;  // Default color is black.

	if( segment.startsWith( '^' ) && segment.length() > 1 )
	{
		char code = segment[ 1 ].toLatin1();
		switch( code )
		{
			case '0': color = Qt::black; break;
			case '1': color = Qt::red; break;
			case '2': color = Qt::green; break;
			case '3': color = Qt::yellow; break;
			case '4': color = Qt::blue; break;
			case '5': color = Qt::cyan; break;
			case '6': color = QColor( "#FF69B4" ); break; // Pink
			case '7': color = DefaultOutputColor; break;
			case '8': color = QColor( "#ADD8E6" ); break; // Light Blue
			case '9': color = QColor( "#FFA500" ); break; // Orange
			default: color = DefaultOutputColor; break; // Default
		}

		// Remove the color code from the segment
		segment.remove( 0, 2 );
	}

	// Set the text color and insert the segment.
	QTextCharFormat format;
	format.setForeground( color );
	cursor.setCharFormat( format );
	cursor.insertText( segment );
}

QColor MLXPlainTextEdit::getSelectedTextColor()
{
	// Get the current cursor (which includes selection information)
	QTextCursor cursor = this->textCursor();

	// If there is no selection, return an invalid color (e.g., transparent)
	if( !cursor.hasSelection() )
	{
		return QColor();  // Returns an invalid color
	}

	// Get the character format for the selected text
	QTextCharFormat format = cursor.charFormat();

	// Return the foreground (text) color of the selected text
	return format.foreground().color();
}

