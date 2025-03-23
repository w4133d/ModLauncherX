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
#include "MLXOutputHighlighter.h"

/// Ran once for each modified line of the output
void MLXOutputHighlighter::highlightBlock( const QString &text )
{
	// Color timestamp with a darker grey
	QRegularExpressionMatch timestamp = QRegularExpression( R"(^(.*?)\|>)" ).match( text );
	if( timestamp.hasMatch() )
		setFormat( timestamp.capturedStart(), timestamp.capturedEnd(), Qt::darkGray );

	// Parse for color codes
	QRegularExpression expression( R"(\^([^^]*))" );
	QRegularExpressionMatchIterator iterator = expression.globalMatch( text );

	// This will run for as many ^'s there are in the line
	while( iterator.hasNext() )
	{
		QRegularExpressionMatch match = iterator.next();
		QString _text = match.captured();
		QColor color;

		// Check which color we need to change to
		switch( _text[ 1 ].toLatin1() )
		{
			case '0': color = Qt::black; break;
			case '1': color = Qt::red; break;
			case '2': color = Qt::green; break;
			case '3': color = QColor( "#D4A900" ); break;
			case '4': color = Qt::blue; break;
			case '5': color = Qt::cyan; break;
			case '6': color = QColor( "#FF69B4" ); break; // Pink
			//case '7': color = default_color; break;
			case '8': color = QColor( "#ADD8E6" ); break; // Light Blue
			case '9': color = QColor( "#FFA500" ); break; // 
			default: color = default_color; break;
		}
		
		setFormat( match.capturedStart(), match.capturedStart() + 2, default_color );
		setFormat( match.capturedStart() + 2, match.capturedEnd(), color );
	}
}
