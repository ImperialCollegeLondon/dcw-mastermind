%!PS-Adobe-3.0
%%Pages: 1
%%EndComments

<< /PageSize [595 842] >> setpagedevice

%include "pslib/debug"
%include "pslib/textsize"
%include "pslib/odd"
%include "pslib/max"
%include "pslib/colours"

/maxguesses 4   def
/numholes   5   def

/g1 (RGBYP) def
/b1 2 def
/w1 2 def

/g2 (RRBBP) def
/b2 3 def
/w2 0 def


%include "mmsupport.i"


%%Page: 1 1
%%PageOrientation: Portrait


debugorigin

maxguesses shownumbers

% say "go 1: guess $g1, score $b1 $w1";
(go 1: guess ) [ g1 (, score ) b1 ( ) w1 () ] debugsay

% say "go 2: guess $g2, score $b2 $w2";
(go 2: guess ) [ g2 (, score ) b2 ( ) w2 () ] debugsay

[ g1 g2 (     ) ]    showguesses

[ b1 w1 b2 w2 ]    showscores

showpage
