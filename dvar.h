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

#pragma once

enum DvarType
{
	DVAR_VALUE_BOOL,
	DVAR_VALUE_INT,
	DVAR_VALUE_STRING
};

struct dvar_s
{
	const char *name;
	const char *description;
	DvarType type;
	int minValue;
	int maxValue;
	bool isCmd;
};

class Dvar
{
	private:
	dvar_s dvar;

	public:
	Dvar();
	Dvar( dvar_s, QTreeWidget * );
	~Dvar();

	static QString setDvarSetting( dvar_s, QCheckBox * );
	static QString setDvarSetting( dvar_s, QSpinBox * );
	static QString setDvarSetting( dvar_s, QLineEdit * );

	static dvar_s findDvar( QString, QTreeWidget *, dvar_s *, int );
};
