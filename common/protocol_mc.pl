#!/usr/bin/perl

$initializeHash = '';
$itemId = 1000;

$headerfileBuffer = '';

open(idfile, ">protocol_item_ids.h");

open(headerfile, ">protocol_items.h");
print headerfile "#ifndef PROTOCOL_ITEMS_H\n"
	. "#define PROTOCOL_ITEMS_H\n\n"
	. "#include \"protocol.h\"\n\n";

open(cppfile, ">protocol_items.cpp");
print cppfile "#include \"protocol.h\"\n"
	. "#include \"protocol_items.h\"\n\n";

open(file, "protocol_items.dat");
while (<file>) {
	s/\s+$//;
	@line = split(/:/);
	$type = shift(@line);
	$name1 = shift(@line);
	($name2 = $name1) =~ s/_(.)/\U$1\E/g;
	$name2 =~ s/^(.)/\U$1\E/;
	if ($type == 0) {
		$type = 'cmd';
		$namePrefix = 'Command';
		$baseClass = 'Command';
		$parentConstructorCall = "$baseClass(\"$name1\")";
		$constructorParamsH = "";
		$constructorParamsCpp = "";
	} elsif ($type == 1) {
		$type = 'cmd';
		$namePrefix = 'Command';
		$baseClass = 'ChatCommand';
		$parentConstructorCall = "$baseClass(\"$name1\", _channel)";
		$constructorParamsH = "const QString &_channel = QString()";
		$constructorParamsCpp = "const QString &_channel";
	} elsif ($type == 2) {
		$type = 'cmd';
		$namePrefix = 'Command';
		$baseClass = 'GameCommand';
		$parentConstructorCall = "$baseClass(\"$name1\", _gameId)";
		$constructorParamsH = "int _gameId = -1";
		$constructorParamsCpp = "int _gameId";
	} elsif ($type == 3) {
		$type = 'game_event';
		$namePrefix = 'Event';
		$baseClass = 'GameEvent';
		$parentConstructorCall = "$baseClass(\"$name1\", _gameId, _playerId)";
		$constructorParamsH = "int _gameId = -1, int _playerId = -1";
		$constructorParamsCpp = "int _gameId, int _playerId";
	} elsif ($type == 4) {
		$type = 'generic_event';
		$namePrefix = 'Event';
		$baseClass = 'GenericEvent';
		$parentConstructorCall = "$baseClass(\"$name1\")";
		$constructorParamsH = "";
		$constructorParamsCpp = "";
	} elsif ($type == 5) {
		$type = 'chat_event';
		$namePrefix = 'Event';
		$baseClass = 'ChatEvent';
		$parentConstructorCall = "$baseClass(\"$name1\", _channel)";
		$constructorParamsH = "const QString &_channel = QString()";
		$constructorParamsCpp = "const QString &_channel";
	}
	$className = $namePrefix . '_' . $name2;
	$itemEnum .= "ItemId_$className = " . ++$itemId . ",\n";
	$headerfileBuffer .= "class $className : public $baseClass {\n"
		. "\tQ_OBJECT\n";
	$constructorCode = '';
	$getFunctionCode = '';
	while ($param = shift(@line)) {
		($key, $value) = split(/,/, $param);
		($prettyVarName = $value) =~ s/_(.)/\U$1\E/g;
		if (!($constructorParamsH eq '')) {
			$constructorParamsH .= ', ';
		}
		if (!($constructorParamsCpp eq '')) {
			$constructorParamsCpp .= ', ';
		}
		($prettyVarName2 = $prettyVarName) =~ s/^(.)/\U$1\E/;
		if ($key eq 'b') {
			$dataType = 'bool';
			$constructorParamsH .= "bool _$prettyVarName = false";
			$constructorParamsCpp .= "bool _$prettyVarName";
			$constructorCode .= "\tinsertItem(new SerializableItem_Bool(\"$value\", _$prettyVarName));\n";
			$getFunctionCode .= "\t$dataType get$prettyVarName2() const { return static_cast<SerializableItem_Bool *>(itemMap.value(\"$value\"))->getData(); };\n";
		} elsif ($key eq 's') {
			$dataType = 'QString';
			$constructorParamsH .= "const QString &_$prettyVarName = QString()";
			$constructorParamsCpp .= "const QString &_$prettyVarName";
			$constructorCode .= "\tinsertItem(new SerializableItem_String(\"$value\", _$prettyVarName));\n";
			$getFunctionCode .= "\t$dataType get$prettyVarName2() const { return static_cast<SerializableItem_String *>(itemMap.value(\"$value\"))->getData(); };\n";
		} elsif ($key eq 'i') {
			$dataType = 'int';
			$constructorParamsH .= "int _$prettyVarName = -1";
			$constructorParamsCpp .= "int _$prettyVarName";
			$constructorCode .= "\tinsertItem(new SerializableItem_Int(\"$value\", _$prettyVarName));\n";
			$getFunctionCode .= "\t$dataType get$prettyVarName2() const { return static_cast<SerializableItem_Int *>(itemMap.value(\"$value\"))->getData(); };\n";
		} elsif ($key eq 'c') {
			$dataType = 'QColor';
			$constructorParamsH .= "const QColor &_$prettyVarName = QColor()";
			$constructorParamsCpp .= "const QColor &_$prettyVarName";
			$constructorCode .= "\tinsertItem(new SerializableItem_Color(\"$value\", _$prettyVarName));\n";
			$getFunctionCode .= "\t$dataType get$prettyVarName2() const { return static_cast<SerializableItem_Color *>(itemMap.value(\"$value\"))->getData(); };\n";
		}
	}
	$headerfileBuffer .= "public:\n"
		. "\t$className($constructorParamsH);\n"
		. $getFunctionCode
		. "\tstatic SerializableItem *newItem() { return new $className; }\n"
		. "\tint getItemId() const { return ItemId_$className; }\n"
		. "};\n";
	print cppfile $className . "::$className($constructorParamsCpp)\n"
		. "\t: $parentConstructorCall\n"
		. "{\n"
		. $constructorCode
		. "}\n";
	$initializeHash .= "\titemNameHash.insert(\"$type$name1\", $className" . "::newItem);\n";
}
close(file);

print idfile "enum AutoItemId {\n"
	. $itemEnum
	. "ItemId_Other = " . ++$itemId . "\n"
	. "};\n";
close(idfile);
	
print headerfile $headerfileBuffer
	. "\n#endif\n";
close(headerfile);

print cppfile "void ProtocolItem::initializeHashAuto()\n"
	. "{\n"
	. $initializeHash
	. "}\n";
close(cppfile);
