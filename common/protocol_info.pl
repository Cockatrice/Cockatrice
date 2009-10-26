#!/usr/bin/perl

open(file, "protocol_info.dat");
while (<file>) {
	s/\s+$//;
	@line = split(/:/);
	$type = shift(@line);
	if ($type == 0) {
		$baseClass = 'Command';
	} elsif ($type == 1) {
		$baseClass = 'ChatCommand';
	} else {
		$baseClass = 'GameCommand';
	}

	$name1 = shift(@line);
	$className = 'Command_' . shift(@line);
	print "class $className : public $baseClass {\n"
		. "private:\n";
	$paramStr1 = '';
	$paramStr2 = '';
	$paramStr3 = '';
	$paramStr4 = '';
	while ($param = shift(@line)) {
		($key, $value) = split(/,/, $param);
		$prettyVarName = $value;
		if (!($paramStr1 eq '')) {
			$paramStr1 .= ', ';
		}
		$paramStr2 .= ", $prettyVarName(_$prettyVarName)";
		$paramStr3 .= "\t\tparameters.insert(\"$value\", $prettyVarName);\n";
		if ($key == 'b') {
			$dataType = 'bool';
			$paramStr1 .= "bool _$prettyVarName = false";
		} elsif ($key == 's') {
			$dataType = 'QString';
			$paramStr1 .= "const QString &_$prettyVarName = QString()";
		} elsif ($key == 'i') {
			$dataType = 'int';
			$paramStr1 .= "int _$prettyVarName = -1";
		}
		$first = substr($prettyVarName, 0, 1);
		$first =~ tr/a-z/A-Z/;
		$prettyVarName2 = $first . substr($prettyVarName, 1, length($prettyVarName));
		$paramStr4 .= "\t$dataType get$prettyVarName2() const { return $prettyVarName; }\n";
		print "\t$dataType $value;\n";
	}
	print "public:\n";
	print "\t$className($paramStr1)\n\t\t: $baseClass(\"$name1\")$paramStr2\n"
		. "\t{\n";
	print $paramStr3;
	print "\t}\n";
	print $paramStr4;
	print "};\n";
}
close(file);
