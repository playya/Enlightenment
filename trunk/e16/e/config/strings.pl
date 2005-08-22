#!/usr/bin/perl

# Script to extract localizable strings from config files
# $ perl strings.pl actionclasses.cfg bindings.cfg menus.cfg >strings.c

sub x() {

  # actionclasses.cfg
  $TTT = 0;
  $TAT = 0;

  if (/__TOOLTIP_TEXT\s+\"(.*)\"/) {
  # print "_(\"$1\"),\n"
    if ($ttt) {
      $ttt = "$ttt\\n$1";
    } else {
      $ttt = "$1";
    }
    $TTT = 1;
  } elsif (/__TOOLTIP_ACTION_TEXT\s+\"(.*)\"/) {
  # print "_(\"$1\"),\n";
    $tat = "$1";
    $TAT = 1;
  }

  if (!($TTT) && $ttt) {
    print "_(\"$ttt\"),\n";
    $ttt = "";
  }

  if (!($TAT) && $tat) {
    print "_(\"$tat\"),\n";
    $tat = "";
  }

  # bindings.cfg
  $TDT = 0;

  if (/^Tooltip\s+(.*)$/) {
  # print "_(\"$1\"),\n"
    if ($tdt) {
      $tdt = "$tdt\\n$1";
    } else {
      $tdt = "$1";
    }
    $TDT = 1;
  }

  if (!($TDT) && $tdt) {
    print "_(\"$tdt\"),\n";
    $tdt = "";
  }

  # menus.cfg
  if (/ADD_MENU_TEXT_ITEM\s*\(\s*(\".*\"),/) {
    $t = "$1";
    print "_($t),\n";
  }
}

#
# Start
#
print "#define _(x) x\n\n";
print "const char *txt[] = {\n";

for ($i=0; $i <= $#ARGV; $i++) {
  $f = $ARGV[$i];
  print "/* $f */\n";
  open F, $f;
  for (<F>) {
     x();
  }
  close $f;
}

print "};\n";
