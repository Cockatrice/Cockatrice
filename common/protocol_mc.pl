#!/usr/bin/perl

$initializeHash = '';

open(headerfile, ">protocol_commands.h");
print headerfile "#ifndef PROTOCOL_COMMANDS_H\n"
	. "#define PROTOCOL_COMMANDS_H\n\n"
	. "#include \"protocol.h\"\n\n";

open(cppfile, ">protocol_commands.cpp");
print cppfile "#include \"protocol.h\"\n"
	. "#include \"protocol_commands.h\"\n\n";

open(file, "protocol_commands.dat");
while (<file>) {
	s/\s+$//;
	@line = split(/:/);
	$type = shift(@line);
	$name1 = shift(@line);
	($name2 = $name1) =~ s/_(.)/\U$1\E/g;
	$name2 =~ s/^(.)/\U$1\E/;
	if ($type == 0) {
		$baseClass = 'Command';
		$parentConstructorCall = "$baseClass(\"$name1\")";
		$constructorParamsH = "";
		$constructorParamsCpp = "";
	} elsif ($type == 1) {
		$baseClass = 'ChatCommand';
		$parentConstructorCall = "$baseClass(\"$name1\", _channel)";
		$constructorParamsH = "const QString &_channel = QString()";
		$constructorParamsCpp = "const QString &_channel";
	} else {
		$baseClass = 'GameCommand';
		$parentConstructorCall = "$baseClass(\"$name1\", _gameId)";
		$constructorParamsH = "int _gameId = -1";
		$constructorParamsCpp = "int _gameId";
	}

	$className = 'Command_' . $name2;
	print headerfile "class $className : public $baseClass {\n"
		. "\tQ_OBJECT\n"
		. "private:\n";
	$paramStr2 = '';
	$paramStr3 = '';
	$paramStr4 = '';
	$paramStr5 = '';
	while ($param = shift(@line)) {
		($key, $value) = split(/,/, $param);
		($prettyVarName = $value) =~ s/_(.)/\U$1\E/g;
		if (!($constructorParamsH eq '')) {
			$constructorParamsH .= ', ';
		}
		if (!($constructorParamsCpp eq '')) {
			$constructorParamsCpp .= ', ';
		}
		$paramStr2 .= ", $prettyVarName(_$prettyVarName)";
		$paramStr3 .= "\tsetParameter(\"$value\", $prettyVarName);\n";
		if ($key eq 'b') {
			$dataType = 'bool';
			$constructorParamsH .= "bool _$prettyVarName = false";
			$constructorParamsCpp .= "bool _$prettyVarName";
			$paramStr5 .= "\t$prettyVarName = (parameters[\"$value\"] == \"1\");\n";
		} elsif ($key eq 's') {
			$dataType = 'QString';
			$constructorParamsH .= "const QString &_$prettyVarName = QString()";
			$constructorParamsCpp .= "const QString &_$prettyVarName";
			$paramStr5 .= "\t$prettyVarName = parameters[\"$value\"];\n";
		} elsif ($key eq 'i') {
			$dataType = 'int';
			$constructorParamsH .= "int _$prettyVarName = -1";
			$constructorParamsCpp .= "int _$prettyVarName";
			$paramStr5 .= "\t$prettyVarName = parameters[\"$value\"].toInt();\n";
		}
		($prettyVarName2 = $prettyVarName) =~ s/^(.)/\U$1\E/;
		$paramStr4 .= "\t$dataType get$prettyVarName2() const { return $prettyVarName; }\n";
		print headerfile "\t$dataType $prettyVarName;\n";
	}
	print headerfile "public:\n"
		. "\t$className($constructorParamsH);\n"
		. $paramStr4
		. "\tstatic Command *newCommand() { return new $className; }\n"
		. ($paramStr5 eq '' ? '' : "protected:\n\tvoid extractParameters();\n")
		. "};\n";
	print cppfile $className . "::$className($constructorParamsCpp)\n"
		. "\t: $parentConstructorCall$paramStr2\n"
		. "{\n"
		. $paramStr3
		. "}\n";
	if (!($paramStr5 eq '')) {
		print cppfile "void $className" . "::extractParameters()\n"
			. "{\n"
			. "\t$baseClass" . "::extractParameters();\n"
			. $paramStr5
			. "}\n";
	}
	$initializeHash .= "\tcommandHash.insert(\"$name1\", $className" . "::newCommand);\n";
}
close(file);

print headerfile "\n#endif\n";
close(headerfile);

print cppfile "void Command::initializeHash()\n"
	. "{\n"
	. $initializeHash
	. "}\n";
close(cppfile);
