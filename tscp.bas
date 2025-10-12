'   TSCP.BAS
'   Tom Kerrigan's Simple Chess Program (TSCP)
'
'   Copyright 2019 Tom Kerrigan
'   port to MMBasic by ceptimus October 2019

Option explicit:Option default integer:Option base 0
Dim MMBasic_version$="version 0.9e 29/11/19"

' DOS version of MMBasic doesn't support Peek and Poke
' (which are used here only to emulate a C union of four bytes
' occupying the same memory as a single 32-bit integer)
' so for use in DOS MMBasic here are some rough equivalents
' written in MMBasic.  Note that these require the addition
' of a comma following the VAR keyword in every call
' Peek supports peeking from integer arrays, but Poke
' only works on non-array integers.
' for stand-alone Micromite and Maximite boards the following
' kluges must be commented out, and the VAR, commas all removed
Dim VAR

Function Peek(dummy,v%,o%)
    Local div(3)=(&h1,&h100,&h10000,&h1000000)

    peek=(v%\div(o%))And &hFF
End Function

Sub Poke dummy,v%,o%,p%
    Local mul(3)=(&h1,&h100,&h10000,&h1000000)
    Local mask(3)=(&hFFFFFF00,&hFFFF00FF,&hFF00FFFF,&hFFFFFF)

    v%=(v% And mask(o%))Or(p%*mul(o%))
End Sub
' end of DOS kluges

' DOS version also doesn't support Library so all the .BAS files
' have to be combined into one huge unwieldy monster.  I kept the
' original individual file headings though.
'=================================================================
'   BOARD.BAS
'   Tom Kerrigan's Simple Chess Program (TSCP)
'
'   Copyright 1997 Tom Kerrigan
'   port to MMBasic by ceptimus October 2019

' init_board() sets the board to the initial game state.
Sub init_board
    Local i

    Restore data_init_hue:For i=0 To 63:Read hue(i):Next i
    Restore data_init_piece:For i=0 To 63:Read piece(i):Next i
    side=LIGHT
    xside=DARK
    castle=15
    ep=-1
    fifty=0
    ply=0
    hply=0
    set_hash ' init_hash() must be called before this function.
    first_move(0)=0
End Sub

' init_hash() initializes the random numbers used by set_hash().
Sub init_hash
    Local i,j,k

    If Left$(MM.Device$,3)<>"ARM" Then Randomize (Timer)
    For i=0 To 1
        For j=0 To 5
            For k=0 To 63
                hash_piece(i,j,k)=hash_rand()
            Next k
        Next j
    Next i
    hash_side=hash_rand()
    For i=0 To 63
        hash_ep(i)=hash_rand()
    Next i
End Sub

' hash_rand() XORs some shifted random numbers together to make sure
' we have good coverage of all 32 bits. (rand() returns 16-bit numbers
' on some systems.)
Function hash_rand()
    Local i,r=0

    For i=0 To 31:r=r Xor ((2^16)*Rnd())*(2^i):Next i
    hash_rand=r
End Function

' set_hash() uses the Zobrist method of generating a unique number (hash)
' for the current chess position. Of course, there are many more chess
' positions than there are 32 bit numbers, so the numbers generated are
' not really unique, but they're unique enough for our purposes (to detect
' repetitions of the position).
' The way it works is to XOR random numbers that correspond to features of
' the position, e.g., if there's a black knight on B8, hash is XORed with
' hash_piece[BLACK][KNIGHT][B8]. All of the pieces are XORed together,
' hash_side is XORed if it's black's move, and the en passant square is
' XORed if there is one. (A chess technicality is that one position can't
' be a repetition of another if the en passant state is different.)
Sub set_hash
    Local i

    hash=0 'hash is global
    For i=0 To 63
        If hue(i)<>EMPTY Then hash=hash Xor hash_piece(hue(i),piece(i),i)
    Next
    If side=DARK Then hash=hash Xor hash_side
    If ep<>-1 Then hash=hash Xor hash_ep(ep)
End Sub

' in_check() returns TRUE if side s is in check and FALSE
' otherwise. It just scans the board to find side s's king
' and calls attack() to see if it's being attacked.
Function in_check(s)
    Local i

    in_check=1 ' in case king not found (shouldn't happen)
    For i=0 To 63
        If (piece(i)=KING)And(hue(i)=s) Then
            in_check=attack(i, s Xor 1)
            i=63
        EndIf
    Next i
End Function

' attack() returns TRUE if square sq is being attacked by side
' s and FALSE otherwise.
Function attack(sq,s)
    Local i,j,n

    attack=0 ' default to false

    For i=0 To 63
        If hue(i)=s Then
            If piece(i)=PAWN Then
                If s=LIGHT Then
                    If ((i And 7)<>0)And(i-9=sq) Then
                        attack=1:Exit For
                    EndIf
                    If ((i And 7)<>7)And (i-7=sq) Then
                        attack=1:Exit For
                    EndIf
                Else
                    If ((i And 7)<>0)And(i+7=sq) Then
                        attack=1:Exit For
                    EndIf
                    If ((i And 7)<>7)And(i+9=sq) Then
                        attack=1:Exit For
                    EndIf
                EndIf
            Else
                For j=0 To offsets(piece(i))-1
                    n=i
                    Do
                        n=mailbox(mailbox64(n)+offset(piece(i),j))
                        If n=-1 Then Exit Do
                        If n=sq Then
                            attack=1:j=offsets(piece(i))-1
                            i=63:Exit Do
                        EndIf
                        If hue(n)<>EMPTY Then Exit Do
                        If slide(piece(i))=0 Then Exit Do
                    Loop
                Next j
            EndIf
        EndIf
    Next i
End Function

' gen() generates pseudo-legal moves for the current position.
' It scans the board to find friendly pieces and then determines
' what squares they attack. When it finds a piece/square
' combination, it calls gen_push to put the move on the "move
' stack."
Sub gen
    Local i,j,n

    ' so far, we have no moves for the current ply
    first_move(ply+1)=first_move(ply)

    For i=0 To 63
        If hue(i)=side Then
            If piece(i)=PAWN Then
                If side=LIGHT Then
                    If (i And 7)<>0 Then
                        If hue(i-9)=DARK Then gen_push(i,i-9,17)
                    EndIf
                    If (i And 7)<>7 Then
                        If hue(i-7)=DARK Then gen_push(i,i-7,17)
                    EndIf
                    If hue(i-8)=EMPTY Then
                        gen_push(i,i-8,16)
                        If i>=48 Then
                            If hue(i-16)=EMPTY Then gen_push(i,i-16,24)
                        EndIf
                    EndIf
                Else
                    If (i And 7)<>0 Then
                        If hue(i+7)=LIGHT Then gen_push(i,i+7,17)
                    EndIf
                    If (i And 7)<>7 Then
                        If hue(i+9)=LIGHT Then gen_push(i,i+9,17)
                    EndIf
                    If hue(i+8)=EMPTY Then
                        gen_push(i,i+8,16)
                        If i<=15 Then
                            If hue(i+16)=EMPTY Then gen_push(i,i+16,24)
                        EndIf
                    EndIf
                EndIf
            Else
                For j=0 To offsets(piece(i))-1
                    n=i
                    Do
                        n=mailbox(mailbox64(n)+offset(piece(i),j))
                        If n=-1 Then Exit Do
                        If hue(n)<>EMPTY Then
                            If hue(n)=xside Then gen_push(i,n,1)
                            Exit Do
                        EndIf
                        gen_push(i,n,0)
                        If slide(piece(i))=0 Then Exit Do
                    Loop
                Next j
            EndIf
        EndIf
    Next i

    ' generate castle moves
    If side=LIGHT Then
        If (castle And 1)<>0 Then gen_push(E1,G1,2)
        If (castle And 2)<>0 Then gen_push(E1,C1,2)
    Else
        If (castle And 4)<>0 Then gen_push(E8,G8,2)
        If (castle And 8)<>0 Then gen_push(E8,C8,2)
    EndIf

    ' generate en passant moves
    If ep<>-1 Then
        If side=LIGHT Then
            If ((ep And 7)<>0)And(hue(ep+7)=LIGHT)And(piece(ep+7)=PAWN) Then
                gen_push(ep+7,ep,21)
            EndIf
            If ((ep And 7)<>7)And(hue(ep+9)=LIGHT)And(piece(ep+9)=PAWN) Then
                gen_push(ep+9,ep,21)
            EndIf
        Else
            If ((ep And 7)<>0)And(hue(ep-9)=DARK)And(piece(ep-9)=PAWN) Then
                gen_push(ep-9,ep,21)
            EndIf
            If ((ep And 7)<>7)And(hue(ep-7)=DARK)And(piece(ep-7)=PAWN) Then
                gen_push(ep-7,ep,21)
            EndIf
        EndIf
    EndIf
End Sub

' gen_caps() is basically a copy of gen() that's modified to
'  only generate capture and promote moves. It's used by the
'  quiescence search.
Sub gen_caps
    Local i,j,n

    first_move(ply+1)=first_move(ply)
    For i=0 To 63
        If hue(i)=side Then
            If piece(i)=PAWN Then
                If side=LIGHT Then
                    If (i And 7)<>0 Then
                        If hue(i-9)=DARK Then gen_push(i,i-9,17)
                    EndIf
                    If (i And 7)<>7 Then
                        If hue(i-7)=DARK Then gen_push(i,i-7,17)
                    EndIf
                    If i<=15 Then
                        If hue(i-8)=EMPTY) Then gen_push(i,i-8,16)
                    EndIf
                EndIf
                If side=DARK Then
                    If (i And 7)<>0 Then
                        If hue(i+7)=LIGHT Then gen_push(i,i+7,17)
                    EndIf
                    If (i And 7)<>7 Then
                        If hue(i+9)=LIGHT Then gen_push(i,i+9,17)
                    EndIf
                    If i>=48 Then
                        If hue(i+8)=EMPTY Then gen_push(i,i+8,16)
                    EndIf
                EndIf
            Else
                For j=0 To offsets(piece(i))-1
                    n=i
                    Do
                        n=mailbox(mailbox64(n)+offset(piece(i),j))
                        If n=-1 Then Exit Do
                        If hue(n)<>EMPTY Then
                            If hue(n)=xside Then gen_push(i,n,1)
                            Exit Do
                        EndIf
                        If slide(piece(i))=0 Then Exit Do
                    Loop
                Next j
            EndIf
        EndIf
    Next i
    If ep<>-1 Then
        If side=LIGHT Then
            If ((ep And 7)<>0)And(hue(ep+7)=LIGHT)And(piece(ep+7)=PAWN) Then
                gen_push(ep+7,ep,21)
            EndIf
            If ((ep And 7)<>7)And(hue(ep+9)=LIGHT)And(piece(ep+9)=PAWN) Then
                gen_push(ep+9,ep,21)
            EndIf
        Else
            If ((ep And 7)<>0)And(hue(ep-9)=DARK)And(piece(ep-9)=PAWN) Then
                gen_push(ep-9,ep,21)
            EndIf
            If ((ep And 7)<>7)And(hue(ep-7)=DARK)And(piece(ep-7)=PAWN) Then
                gen_push(ep-7,ep,21)
            EndIf
        EndIf
    EndIf
End Sub

' gen_push() puts a move on the move stack, unless it's a
' pawn promotion that needs to be handled by gen_promote().
' It also assigns a score to the move for alpha-beta move
' ordering. If the move is a capture, it uses MVV/LVA
' (Most Valuable Victim/Least Valuable Attacker). Otherwise,
' it uses the move's history heuristic value. Note that
' 1,000,000 is added to a capture move's score, so it
' always gets ordered above a "normal" move.
Sub gen_push(from,too,bits)
    Local g,p

    If (bits And 16)<>0 Then
        If side=LIGHT) Then
            If too<=H8 Then gen_promote(from,too,bits):Exit Sub
        Else
            If too>=A1 Then gen_promote(from,too,bits):Exit Sub
        EndIf
    EndIf
    g=first_move(ply+1)
    first_move(ply+1)=first_move(ply+1)+1
    poke VAR, p,0,from
    poke VAR, p,1,too
    poke VAR, p,2,0 'promote default
    poke VAR, p,3,bits
    gen_dat(g,0)=p

    If hue(too)<>EMPTY Then
        gen_dat(g,1)=1000000+piece(too)*10-piece(from)
    Else
        gen_dat(g,1)=history(from,too)
    EndIf
End Sub

' gen_promote() is just like gen_push(), only it puts 4 moves
' on the move stack, one for each possible promotion piece
Sub gen_promote(from,too,bits)
    Local i,g,p

    For i=KNIGHT To QUEEN
        g=first_move(ply+1)
        first_move(ply+1)=first_move(ply+1)+1
        poke VAR, p,0,from
        poke VAR, p,1,too
        poke VAR, p,2,i
        poke VAR, p,3,bits Or 32
        gen_dat(g,0)=p

        gen_dat(g,1)=1000000+i*10
    Next i
End Sub

' makemove() makes a move. If the move is illegal, it
' undoes whatever it did and returns FALSE. Otherwise, it
' returns TRUE.
Function makemove(m)
    ' test to see if a castle move is legal and move the rook
    ' (the king is moved with the usual move code later)
    Local from,too
    Local m_from=peek(VAR, m,0)
    Local m_too=peek(VAR, m,1)
    Local m_promote=peek(VAR, m,2)
    Local m_bits=peek(VAR, m,3)
    Local break=0

    makemove=0 'default false

    If (m_bits And 2)<>0 Then
        If in_check(side)<>0 Then Exit Function
        Select Case m_too
            Case 62
                If (hue(F1)<>EMPTY)Or(hue(G1)<>EMPTY) Then
                    break=1
                ElseIf (attack(F1,xside)<>0)Or(attack(G1,xside)<>0) Then
                    break=1
                EndIf
                from=H1
                too=F1
            Case 58
                If (hue(B1)<>EMPTY)Or(hue(C1)<>EMPTY)Or(hue(D1)<>EMPTY) Then
                    break=1
                ElseIf (attack(C1,xside)<>0)Or(attack(D1,xside)<>0) Then
                    break=1
                EndIf
                from=A1
                too=D1
            Case 6
                If (hue(F8)<>EMPTY)Or(hue(G8)<>EMPTY) Then
                    break=1
                ElseIf (attack(F8,xside)<>0)Or(attack(G8,xside)<>0) Then
                    break=1
                EndIf
                from=H8
                too=F8
            Case 2
                If (hue(B8)<>EMPTY)Or(hue(C8)<>EMPTY)Or(hue(D8)<>EMPTY) Then
                    break=1
                ElseIf (attack(C8,xside)<>0)Or(attack(D8,xside)<>0) Then
                    break=1
                EndIf
                from=A8
                too=D8
            Case Else  ' shouldn't get here
                from=-1
                too=-1
        End Select
        If break<>0 Then Exit Function
        hue(too)=hue(from)
        piece(too)=piece(from)
        hue(from)=EMPTY
        piece(from)=EMPTY
    EndIf

    ' back up information so we can take the move back later.
    hist_dat(hply,0)=m
    hist_dat(hply,1)=piece(m_too)
    hist_dat(hply,2)=castle
    hist_dat(hply,3)=ep
    hist_dat(hply,4)=fifty
    hist_dat(hply,5)=hash
    ply=ply+1
    hply=hply+1

    ' update the castle, en passant, and
    ' fifty-move-draw variables
    castle=castle And (castle_mask(m_from) And castle_mask(m_too))
    If (m_bits And 8)<>0 Then
        If side=LIGHT Then
            ep=m_too+8
        Else
            ep=m_too-8
        EndIf
    Else
        ep=-1
    EndIf
    If (m_bits And 17)<>0 Then
        fifty=0
    Else
        fifty=fifty+1
    EndIf

    ' move the piece
    hue(m_too)=side
    If (m_bits And 32)<>0 Then
        piece(m_too)=m_promote
    Else
        piece(m_too)=piece(m_from)
    EndIf
    hue(m_from)=EMPTY
    piece(m_from)=EMPTY

    ' erase the pawn if this is an en passant move
    If (m_bits And 4)<>0 Then
        If side=LIGHT Then
            hue(m_too+8)=EMPTY
            piece(m_too+8)=EMPTY
        Else
            hue(m_too-8)=EMPTY
            piece(m_too-8)=EMPTY
        EndIf
    EndIf

    ' switch sides and test for legality (if we can capture
    ' the other guy's king, it's an illegal position and
    ' we need to take the move back)
    side=side Xor 1
    xside=xside Xor 1
    If in_check(xside)<>0 Then takeback:Exit Function
    set_hash
    makemove=1
End Function

' takeback() is very similar to makemove(), only backwards :)
Sub takeback
    Local m,m_from,m_too,m_bits,from,too

    side=side Xor 1
    xside=xside Xor 1
    ply=ply-1
    hply=hply-1
    m=hist_dat(hply,0)
    m_from=peek(VAR, m,0):m_too=peek(VAR, m,1):m_bits=peek(VAR, m,3)
    castle=hist_dat(hply,2)
    ep=hist_dat(hply,3)
    fifty=hist_dat(hply,4)
    hash=hist_dat(hply,5)
    hue(m_from)=side
    If (m_bits And 32)<>0 Then
        piece(m_from)=PAWN
    Else
        piece(m_from)=piece(m_too)
    EndIf
    If hist_dat(hply,1)=EMPTY Then
        hue(m_too)=EMPTY
        piece(m_too)=EMPTY
    Else
        hue(m_too)=xside
        piece(m_too)=hist_dat(hply,1)
    EndIf
    If (m_bits And 2)<>0 Then
        Select Case m_too
            Case 62
                from=F1
                too=H1
            Case 58
                from=D1
                too=A1
            Case 6
                from=F8
                too=H8
            Case 2
                from=D8
                too=A8
            Case Else ' shouldn't get here
                from=-1
                too=-1
        End Select
        hue(too)=side
        piece(too)=ROOK
        hue(from)=EMPTY
        piece(from)=EMPTY
    EndIf
    If (m_bits And 4)<>0 Then
        If side=LIGHT Then
            hue(m_too+8)=xside
            piece(m_too+8)=PAWN
        Else
            hue(m_too-8)=xside
            piece(m_too-8)=PAWN
        EndIf
    EndIf
End Sub
'=================================================================
'   BOOK.BAS
'   Tom Kerrigan's Simple Chess Program (TSCP)
'   Copyright 1997 Tom Kerrigan
'   port to MMBasic by ceptimus October 2019

Dim book_file=0 'set to 1 if opening book is present

' open_book() checks if the opening book file is present
' and initializes the random number generator so we play random book moves.
Sub open_book
    Local d$=MM.Device$

    If Len(d$)>2 Then
        If Left$(d$,3)<>"ARM" Then 'Armmites have hardware random and no Randomize
            Randomize Timer
        EndIf
    EndIf

    On Error Skip
    Open "book.txt" For input As#1
    If MM.Errno<>0 Then
        Print "Opening book missing."
    Else
        Close #1
        book_file=1
    EndIf
End Sub

' close_book() is called when the program exits or when running the benchmark
Sub close_book
    book_file=0
End Sub

' book_move() returns a book move (in integer format) or -1 if there is no
' book move.
Function book_move()
    Local line$,book_line$
    Local i,j,m,found
    Local move(50)  'the possible book moves
    Local count(50) 'the number of occurrences of each move
    Local moves,total_count

    moves=0
    total_count=0
    book_move=-1 'default to "no book move"

    If book_file=0 Or hply>25 Then Exit Function

    'line is a string with the current line, e.g., "e2e4 e7e5 g1f3 "
    line$=""

    For i=0 To hply-1
        line$=line$+move_str$(hist_dat(i,0))+" "
    Next i
    'Print "line$=["line$"]"

    ' compare line to each line in the opening book
    Open "book.txt" For input As#1
    Do While Not Eof(#1)
        Input #1, book_line$
        'Print "book_line$=["book_line$"]"
        If book_match(line$, book_line$)<>0 Then
            ' parse the book move that continues the line
            m=parse_move(Mid$(book_line$, Len(line$)+1))
            If m<>-1 Then
                m=gen_dat(m, 0)

                ' add the book move to the move list, or update the move's count
                found=0
                For j=0 To moves-1
                    If move(j)=m Then count(j)=count(j)+1:found=1:j=moves
                Next j

                If found=0 Then
                    move(moves)=m
                    count(moves)=1
                    moves=moves+1
                EndIf
                total_count=total_count+1
            EndIf
        EndIf
    Loop
    Close #1
    If moves=0 Then Exit Function 'no book moves found

    ' Think of total_count as the set of matching book lines.
    ' Randomly pick one of those lines (j) and figure out which
    ' move j "corresponds" to.
    j=Int(Rnd()*total_count)
    For i=0 To moves-1
        j=j-count(i)
        If j<0 Then book_move=move(i):i=moves
    Next i
End Function

'book_match() returns TRUE if the first part of s2 matches s1
Function book_match(s1$, s2$)
    book_match=0
    If Left$(s2$, Len(s1$))=s1$ Then book_match=1
End Function
'=================================================================
'   DEFS.BAS
'   Tom Kerrigan's Simple Chess Program (TSCP)
'
'   Copyright 1997 Tom Kerrigan
'   port to MMBasic by ceptimus October 2019

Const GEN_STACK  = 1120
Const MAX_PLY    = 32
Const HIST_STACK = 400

Const LIGHT      = 0
Const DARK       = 1

Const PAWN       = 0
Const KNIGHT     = 1
Const BISHOP     = 2
Const ROOK       = 3
Const QUEEN      = 4
Const KING       = 5

Const EMPTY      = 6

' useful squares
Const A1 =56
Const B1 =57
Const C1 =58
Const D1 =59
Const E1 =60
Const F1 =61
Const G1 =62
Const H1 =63
Const A8 = 0
Const B8 = 1
Const C8 = 2
Const D8 = 3
Const E8 = 4
Const F8 = 5
Const G8 = 6
Const H8 = 7

' This is the basic description of a move. promote is what
' piece to promote the pawn to, if the move is a pawn
' promotion. bits is a bitfield that describes the move,
' with the following bits:
'
' 1 capture
' 2 castle
' 4 en passant capture
' 8 pushing a pawn 2 squares
' 16    pawn move
' 32    promote
'
' It's union'ed with an integer so two moves can easily
' be compared with each other.
'
' typedef struct {
'   char from;
'   char to;
'   char promote;
'   char bits;
' } move_bytes;
'
' typedef union {
'   move_bytes b;
'   int u;
' } move;
'
' an element of the move stack. it's just a move with a
' score, so it can be sorted by the search functions.
' typedef struct {
'   move m;
'   int score;
' } gen_t;
'
' an element of the history stack, with the information
' necessary to take a move back.
' typedef struct {
'   move m;
'   int capture;
'   int castle;
'   int ep;
'   int fifty;
'   int hash;
' } hist_t;
'=================================================================
'   DATA.BAS
'   Tom Kerrigan's Simple Chess Program (TSCP)
'
'   Copyright 1997 Tom Kerrigan
'   port to MMBasic by ceptimus October 2019

' MMBasic doesn't allow initialization of arrays if the elements
' exceed one program line in length (about 80 characters) so this
' initialization subroutine is called at program start up to read
' values into the larger arrays from data statements
Sub init_arrays
    Local i,j

    Restore data_mailbox:For i=0 To 119:Read mailbox(i):Next i
    Restore data_mailbox64:For i=0 To 63:Read mailbox64(i):Next i
    Restore data_offset
    For i=0 To 5
        For j=0 To 7
            Read offset(i,j)
        Next j
    Next i
    Restore data_castle_mask:For i=0 To 63:Read castle_mask(i):Next i
    Restore data_pawn_pcsq:For i=0 To 63:Read pawn_pcsq(i):Next i
    Restore data_knight_pcsq:For i=0 To 63:Read knight_pcsq(i):Next i
    Restore data_bishop_pcsq:For i=0 To 63:Read bishop_pcsq(i):Next i
    Restore data_king_pcsq:For i=0 To 63:Read king_pcsq(i):Next i
    Restore data_king_endgame_pcsq
    For i=0 To 63:Read king_endgame_pcsq(i):Next i
    Restore data_flip:For i=0 To 63:Read flip(i):Next i
End Sub

' the board representation
Dim hue(63)  ' LIGHT, DARK, or EMPTY
Dim piece(63)  ' PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, or EMPTY
Dim side ' the side to move
Dim xside ' the side not to move
Dim castle  ' a bitfield with the castle permissions. if 1 is set,
            ' white can still castle kingside. 2 is white queenside.
            ' 4 is black kingside. 8 is black queenside.
Dim ep  ' the en passant square. if white moves e2e4, the en passant
        ' square is set to e3, because that's where a pawn would move
        ' in an en passant capture
Dim fifty   ' the number of moves since a capture or pawn move, used
            ' to handle the fifty-move-draw rule
Dim hash    ' a (more or less) unique number that corresponds to the
            ' position
Dim ply ' the number of half-moves (ply) since the
        ' root of the search tree
Dim hply    ' h for history; the number of ply since the beginning
            ' of the game

' gen_dat is some memory for move lists that are created by the move
' generators. The move list for ply n starts at first_move[n] and ends
' at first_move[n + 1].
Dim gen_dat(GEN_STACK-1,1) ' 2nd index 0=move union, 1=score
Dim first_move(MAX_PLY-1)

' the history heuristic array (used for move ordering)
Dim history(63,63)

' we need an array of hist_t's so we can take back the
' moves we make
' 6 ints per entry. First int is also union of 4 bytes:from,to,promote,bits
Dim hist_dat(HIST_STACK-1, 5)

' the engine will search for max_time milliseconds or until it finishes
' searching max_depth ply.
Dim max_time,max_depth

' the time when the engine starts searching, and when it should stop
Dim start_time,stop_time

Dim nodes 'the number of nodes we've searched

' a "triangular" PV array; for a good explanation of why a triangular
' array is needed, see "How Computers Play Chess" by Levy and Newborn.
Dim pv(MAX_PLY-1, MAX_PLY-1) 'each element is a 'move' union: an int or 4 bytes
Dim pv_length(MAX_PLY-1)
Dim follow_pv

' random numbers used to compute hash; see set_hash() in board.bas
Dim hash_piece(1,5,63) ' indexed by piece: color,type,square
Dim hash_side
Dim hash_ep(63)

' Now we have the mailbox array, so called because it looks like a
' mailbox, at least according to Bob Hyatt. This is useful when we
' need to figure out what pieces can go where. Let's say we have a
' rook on square a4 (32) and we want to know if it can move one
' square to the left. We subtract 1, and we get 31 (h5). The rook
' obviously can't move to h5, but we don't know that without doing
' a lot of annoying work. Sooooo, what we do is figure out a4's
' mailbox number, which is 61. Then we subtract 1 from 61 (60) and
' see what mailbox[60] is. In this case, it's -1, so it's out of
' bounds and we can forget it. You can see how mailbox[] is used
' in attack() in board.bas
Dim mailbox(119)
data_mailbox:
Data -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
Data -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
Data -1,  0,  1,  2,  3,  4,  5,  6,  7, -1
Data -1,  8,  9, 10, 11, 12, 13, 14, 15, -1
Data -1, 16, 17, 18, 19, 20, 21, 22, 23, -1
Data -1, 24, 25, 26, 27, 28, 29, 30, 31, -1
Data -1, 32, 33, 34, 35, 36, 37, 38, 39, -1
Data -1, 40, 41, 42, 43, 44, 45, 46, 47, -1
Data -1, 48, 49, 50, 51, 52, 53, 54, 55, -1
Data -1, 56, 57, 58, 59, 60, 61, 62, 63, -1
Data -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
Data -1, -1, -1, -1, -1, -1, -1, -1, -1, -1

Dim mailbox64(63)
data_mailbox64:
Data 21, 22, 23, 24, 25, 26, 27, 28
Data 31, 32, 33, 34, 35, 36, 37, 38
Data 41, 42, 43, 44, 45, 46, 47, 48
Data 51, 52, 53, 54, 55, 56, 57, 58
Data 61, 62, 63, 64, 65, 66, 67, 68
Data 71, 72, 73, 74, 75, 76, 77, 78
Data 81, 82, 83, 84, 85, 86, 87, 88
Data 91, 92, 93, 94, 95, 96, 97, 98

' slide, offsets, and offset are basically the vectors that
' pieces can move in. If slide for the piece is FALSE, it can
' only move one square in any one direction. offsets is the
' number of directions it can move in, and offset is an array
' of the actual directions.
Dim slide(5)=(0,0,1,1,1,0)
Dim offsets(5)=(0,8,4,4,8,8)

Dim offset(5,7)
data_offset:
Data   0,   0,   0,   0,   0,   0,   0,   0
Data -21, -19, -12,  -8,   8,  12,  19,  21
Data -11,  -9,   9,  11,   0,   0,   0,   0
Data -10,  -1,   1,  10,   0,   0,   0,   0
Data -11, -10,  -9,  -1,   1,   9,  10,  11
Data -11, -10,  -9,  -1,   1,   9,  10,  11

' This is the castle_mask array. We can use it to determine
' the castling permissions after a move. What we do is
' logical-AND the castle bits with the castle_mask bits for
' both of the move's squares. Let's say castle is 1, meaning
' that white can still castle kingside. Now we play a move
' where the rook on h1 gets captured. We AND castle with
' castle_mask[63], so we have 1&14, and castle becomes 0 and
' white can't castle kingside anymore.
Dim castle_mask(63)
data_castle_mask:
Data  7, 15, 15, 15,  3, 15, 15, 11
Data 15, 15, 15, 15, 15, 15, 15, 15
Data 15, 15, 15, 15, 15, 15, 15, 15
Data 15, 15, 15, 15, 15, 15, 15, 15
Data 15, 15, 15, 15, 15, 15, 15, 15
Data 15, 15, 15, 15, 15, 15, 15, 15
Data 15, 15, 15, 15, 15, 15, 15, 15
Data 13, 15, 15, 15, 12, 15, 15, 14

' the piece letters, for print_board()
Dim piece_char$="PNBRQKpnbrqk"

' the initial board state
data_init_hue:
Data 1, 1, 1, 1, 1, 1, 1, 1
Data 1, 1, 1, 1, 1, 1, 1, 1
Data 6, 6, 6, 6, 6, 6, 6, 6
Data 6, 6, 6, 6, 6, 6, 6, 6
Data 6, 6, 6, 6, 6, 6, 6, 6
Data 6, 6, 6, 6, 6, 6, 6, 6
Data 0, 0, 0, 0, 0, 0, 0, 0
Data 0, 0, 0, 0, 0, 0, 0, 0

data_init_piece:
Data 3, 1, 2, 4, 5, 2, 1, 3
Data 0, 0, 0, 0, 0, 0, 0, 0
Data 6, 6, 6, 6, 6, 6, 6, 6
Data 6, 6, 6, 6, 6, 6, 6, 6
Data 6, 6, 6, 6, 6, 6, 6, 6
Data 6, 6, 6, 6, 6, 6, 6, 6
Data 0, 0, 0, 0, 0, 0, 0, 0
Data 3, 1, 2, 4, 5, 2, 1, 3

Dim stop_search=0 'flag gets set when thinking time has expired
'=================================================================
'   EVAL.BAS
'   Tom Kerrigan's Simple Chess Program (TSCP)
'
'   Copyright 1997 Tom Kerrigan
'   port to MMBasic by ceptimus October 2019

Const DOUBLED_PAWN_PENALTY      = 10
Const ISOLATED_PAWN_PENALTY     = 20
Const BACKWARDS_PAWN_PENALTY    =  8
Const PASSED_PAWN_BONUS         = 20
Const ROOK_SEMI_OPEN_FILE_BONUS = 10
Const ROOK_OPEN_FILE_BONUS      = 15
Const ROOK_ON_SEVENTH_BONUS     = 20

' the values of the pieces
Dim piece_value(5)=(100,300,300,500,900,0)

' The "pcsq" arrays are piece/square tables. They're values
' added to the material value of the piece based on the
' location of the piece.
Dim pawn_pcsq(63)
data_pawn_pcsq:
Data  0,   0,   0,   0,   0,   0,   0,   0
Data  5,  10,  15,  20,  20,  15,  10,   5
Data  4,   8,  12,  16,  16,  12,   8,   4
Data  3,   6,   9,  12,  12,   9,   6,   3
Data  2,   4,   6,   8,   8,   6,   4,   2
Data  1,   2,   3, -10, -10,   3,   2,   1
Data  0,   0,   0, -40, -40,   0,   0,   0
Data  0,   0,   0,   0,   0,   0,   0,   0

Dim knight_pcsq(63)
data_knight_pcsq:
Data -10, -10, -10, -10, -10, -10, -10, -10
Data -10,   0,   0,   0,   0,   0,   0, -10
Data -10,   0,   5,   5,   5,   5,   0, -10
Data -10,   0,   5,  10,  10,   5,   0, -10
Data -10,   0,   5,  10,  10,   5,   0, -10
Data -10,   0,   5,   5,   5,   5,   0, -10
Data -10,   0,   0,   0,   0,   0,   0, -10
Data -10, -30, -10, -10, -10, -10, -30, -10

Dim bishop_pcsq(63)
data_bishop_pcsq:
Data -10, -10, -10, -10, -10, -10, -10, -10
Data -10,   0,   0,   0,   0,   0,   0, -10
Data -10,   0,   5,   5,   5,   5,   0, -10
Data -10,   0,   5,  10,  10,   5,   0, -10
Data -10,   0,   5,  10,  10,   5,   0, -10
Data -10,   0,   5,   5,   5,   5,   0, -10
Data -10,   0,   0,   0,   0,   0,   0, -10
Data -10, -10, -20, -10, -10, -20, -10, -10

Dim king_pcsq(63)
data_king_pcsq:
Data -40, -40, -40, -40, -40, -40, -40, -40
Data -40, -40, -40, -40, -40, -40, -40, -40
Data -40, -40, -40, -40, -40, -40, -40, -40
Data -40, -40, -40, -40, -40, -40, -40, -40
Data -40, -40, -40, -40, -40, -40, -40, -40
Data -40, -40, -40, -40, -40, -40, -40, -40
Data -20, -20, -20, -20, -20, -20, -20, -20
Data   0,  20,  40, -20,   0, -20,  40,  20

Dim king_endgame_pcsq(63)
data_king_endgame_pcsq:
Data  0,  10,  20,  30,  30,  20,  10,   0
Data 10,  20,  30,  40,  40,  30,  20,  10
Data 20,  30,  40,  50,  50,  40,  30,  20
Data 30,  40,  50,  60,  60,  50,  40,  30
Data 30,  40,  50,  60,  60,  50,  40,  30
Data 20,  30,  40,  50,  50,  40,  30,  20
Data 10,  20,  30,  40,  40,  30,  20,  10
Data  0,  10,  20,  30,  30,  20,  10,   0

' The flip array is used to calculate the piece/square
' values for DARK pieces. The piece/square value of a
' LIGHT pawn is pawn_pcsq[sq] and the value of a DARK
' pawn is pawn_pcsq[flip[sq]]
Dim flip(63)
data_flip:
Data 56,  57,  58,  59,  60,  61,  62,  63
Data 48,  49,  50,  51,  52,  53,  54,  55
Data 40,  41,  42,  43,  44,  45,  46,  47
Data 32,  33,  34,  35,  36,  37,  38,  39
Data 24,  25,  26,  27,  28,  29,  30,  31
Data 16,  17,  18,  19,  20,  21,  22,  23
Data  8,   9,  10,  11,  12,  13,  14,  15
Data  0,   1,   2,   3,   4,   5,   6,   7

' pawn_rank[x][y] is the rank of the least advanced pawn of color x on file
' y - 1. There are "buffer files" on the left and right to avoid special-case
' logic later. If there's no pawn on a rank, we pretend the pawn is
' impossibly far advanced (0 for LIGHT and 7 for DARK). This makes it easy to
' test for pawns on a rank and it simplifies some pawn evaluation code.
Dim pawn_rank(1,9)

Dim piece_mat(1) 'the value of a side's pieces
Dim pawn_mat(1)  'the value of a side's pawns

Function eval_()
    Local i,f,score(1)

    ' this is the first pass: set up pawn_rank, piece_mat, and pawn_mat.
    For i=0 To 9
        pawn_rank(LIGHT,i)=0
        pawn_rank(DARK,i)=7
    Next i
    piece_mat(LIGHT)=0
    piece_mat(DARK)=0
    pawn_mat(LIGHT)=0
    pawn_mat(DARK)=0
    For i=0 To 63
        If hue(i)<>EMPTY Then
            If piece(i)=PAWN Then
                pawn_mat(hue(i))=pawn_mat(hue(i))+piece_value(PAWN)
                f=(i And 7)+1 ' add 1 because of the extra file in the array
                If hue(i)=LIGHT Then
                    If pawn_rank(LIGHT,f)<i\8 Then pawn_rank(LIGHT,f)=i\8
                Else
                    If pawn_rank(DARK,f)>i\8 Then pawn_rank(DARK,f)=i\8
                EndIf
            Else
                piece_mat(hue(i))=piece_mat(hue(i))+piece_value(piece(i))
            EndIf
        EndIf
    Next i

    ' this is the second pass: evaluate each piece
    score(LIGHT)=piece_mat(LIGHT)+pawn_mat(LIGHT)
    score(DARK)=piece_mat(DARK)+pawn_mat(DARK)
    For i=0 To 63
        'this weird construct is because MMBasic has no continue
        Do
            If hue(i)<>EMPTY Then Exit Do
            i=i+1:If i>63 Then Exit Do
        Loop :If i>63 Then Exit For
        If hue(i)=LIGHT Then
            Select Case piece(i)
                Case PAWN
                    score(LIGHT)=score(LIGHT)+eval_light_pawn(i)
                Case KNIGHT
                    score(LIGHT)=score(LIGHT)+knight_pcsq(i)
                Case BISHOP
                    score(LIGHT)=score(LIGHT)+bishop_pcsq(i)
                Case ROOK
                    If pawn_rank(LIGHT,(i And 7)+1)=0 Then
                        If pawn_rank(DARK,(i And 7)+1)=7 Then
                            score(LIGHT)=score(LIGHT)+ROOK_OPEN_FILE_BONUS
                        Else
                            score(LIGHT)=score(LIGHT)+ROOK_SEMI_OPEN_FILE_BONUS
                        EndIf
                    EndIf
                    If i\8=1 Then
                        score(LIGHT)=score(LIGHT)+ROOK_ON_SEVENTH_BONUS
                    EndIf
                Case KING
                    If piece_mat(DARK)<=1200 Then
                        score(LIGHT)=score(LIGHT)+king_endgame_pcsq(i)
                    Else
                        score(LIGHT)=score(LIGHT)+eval_light_king(i)
                    EndIf
            End Select
        Else
            Select Case piece(i)
                Case PAWN
                    score(DARK)=score(DARK)+eval_dark_pawn(i)
                Case KNIGHT
                    score(DARK)=score(DARK)+knight_pcsq(flip(i))
                Case BISHOP
                    score(DARK)=score(DARK)+bishop_pcsq(flip(i))
                Case ROOK
                    If pawn_rank(DARK,(i And 7)+1)=7 Then
                        If pawn_rank(LIGHT,(i And 7)+1)=0 Then
                            score(DARK)=score(DARK)+ROOK_OPEN_FILE_BONUS
                        Else
                            score(DARK)=score(DARK)+ROOK_SEMI_OPEN_FILE_BONUS
                        EndIf
                    EndIf
                    If i\8=6 Then
                        score(DARK)=score(DARK)+ROOK_ON_SEVENTH_BONUS
                    EndIf
                Case KING
                    If piece_mat(LIGHT)<=1200 Then
                        score(DARK)=score(DARK)+king_endgame_pcsq(flip(i))
                    Else
                        score(DARK)=score(DARK)+eval_dark_king(i)
                    EndIf
            End Select
        EndIf
    Next i

    ' the score[] array is set, now return the score relative
    ' to the side to move
    If side=LIGHT Then
        eval_=score(LIGHT)-score(DARK)
    Else
        eval_=score(DARK)-score(LIGHT)
    EndIf
End Function

Function eval_light_pawn(sq)
    Local r ' the value to return
    Local f ' the pawn's file

    r=0
    f=(sq And 7)+1

    r=r+pawn_pcsq(sq)

    ' if there's a pawn behind this one, it's doubled
    If pawn_rank(LIGHT,f)>sq\8 Then r=r-DOUBLED_PAWN_PENALTY

    ' if there aren't any friendly pawns on either side of
    ' this one, it's isolated
    If (pawn_rank(LIGHT,f-1)=0)And(pawn_rank(LIGHT,f+1)=0) Then
        r=r-ISOLATED_PAWN_PENALTY
    ' if it's not isolated, it might be backwards
    ElseIf (pawn_rank(LIGHT,f-1)<sq\8)And(pawn_rank(LIGHT,f+1)<sq\8) Then
        r=r-BACKWARDS_PAWN_PENALTY
    EndIf

    ' add a bonus if the pawn is passed
    If (pawn_rank(DARK,f-1)>=sq\8)And(pawn_rank(DARK,f)>=sq\8) Then
        If pawn_rank(DARK,f+1)>=sq\8 Then
            r=r+(7-(sq\8))*PASSED_PAWN_BONUS
        EndIf
    EndIf

    eval_light_pawn=r
End Function

Function eval_dark_pawn(sq)
    Local r ' the value to return
    Local f ' the pawn's file

    f=(sq And 7)+1
    r=pawn_pcsq(flip(sq))

    ' if there's a pawn behind this one, it's doubled
    If pawn_rank(DARK,f)<sq\8 Then r=r-DOUBLED_PAWN_PENALTY

    ' if there aren't any friendly pawns on either side of
    ' this one, it's isolated
    If (pawn_rank(DARK,f-1)=7)And(pawn_rank(DARK,f+1)=7) Then
        r=r-ISOLATED_PAWN_PENALTY
    ' if it's not isolated, it might be backwards
    ElseIf (pawn_rank(DARK,f-1)>sq\8)And(pawn_rank(DARK,f+1)>sq\8) Then
        r=r-BACKWARDS_PAWN_PENALTY
    EndIf

    ' add a bonus if the pawn is passed
    If (pawn_rank(LIGHT,f-1)<=sq\8)And(pawn_rank(LIGHT,f)<=sq\8) Then
        If pawn_rank(LIGHT,f+1)<=sq\8 Then
            r=r+(sq\8)*PASSED_PAWN_BONUS
        EndIf
    EndIf
    eval_dark_pawn=r
End Function

Function eval_light_king(sq)
    Local r ' the value to return
    Local i

    r=king_pcsq(sq)

    ' if the king is castled, use a special function to evaluate the
    ' pawns on the appropriate side
    If (sq And 7)<3 Then
        r=r+eval_lkp(1)
        r=r+eval_lkp(2)
        r=r+eval_lkp(3)\2 ' problems with pawns on the c & f files
                          ' are Not As severe
    ElseIf (sq And 7)>4 Then
        r=r+eval_lkp(8)
        r=r+eval_lkp(7)
        r=r+eval_lkp(6)\2

    ' otherwise, just assess a penalty if there are open files near
    ' the king
    Else
        For i=(sq And 7) To (sq And 7)+2
            If (pawn_rank(LIGHT,i)=0)And(pawn_rank(DARK,i)=7) Then
                r=r-10
            EndIf
        Next i
    EndIf

    ' scale the king safety value according to the opponent's material;
    ' the premise is that your king safety can only be bad if the
    ' opponent has enough pieces to attack you
    r=r*piece_mat(DARK)
    r=r\3100

    eval_light_king=r
End Function

' eval_lkp(f) evaluates the Light King Pawn on file f
Function eval_lkp(f)
    Local r=0

    If pawn_rank(LIGHT,f)=6 Then
        ' pawn hasn't moved
    ElseIf pawn_rank(light,f)=5 Then
        r=r-10  ' pawn moved one square
    ElseIf pawn_rank(LIGHT,f)<>0 Then
        r=r-20 ' pawn moved more than one square
    Else
        r=r-25 ' no pawn on this file
    EndIf

    If  pawn_rank(DARK,f)=7 Then
        r=r-15 ' no enemy pawn
    ElseIf pawn_rank(DARK,f)=5 Then
        r=r-10 ' enemy pawn on the 3rd rank
    ElseIf pawn_rank(DARK,f)=4 Then
        r=r-5 ' enemy pawn on the 4th rank
    EndIf

    eval_lkp=r
End Function

Function eval_dark_king(sq)
    Local r,i

    r=king_pcsq(flip(sq))
    If (sq And 7)<3 Then
        r=r+eval_dkp(1)
        r=r+eval_dkp(2)
        r=r+eval_dkp(3)\2
    ElseIf (sq And 7)>4 Then
        r=r+eval_dkp(8)
        r=r+eval_dkp(7)
        r=r+eval_dkp(6)\2
    Else
        For i=(sq And 7) To (sq And 7)+2
            If (pawn_rank(LIGHT,i)=0)And(pawn_rank(DARK,i)=7) Then
                r=r-10
            EndIf
        Next i
    EndIf

    r=r*piece_mat(LIGHT)
    r=r\3100
    eval_dark_king=r
End Function

Function eval_dkp(f)
    Local r=0

    If pawn_rank(DARK,f)=1 Then
        ' pawn hasn't moved
    ElseIf pawn_rank(DARK,f)=2 Then
        r=r-10
    ElseIf pawn_rank(DARK,f)<>7 Then
        r=r-20
    Else
        r=r-25
    EndIf

    If pawn_rank(LIGHT,f)=0 Then
        r=r-15
    ElseIf pawn_rank(LIGHT,f)=2 Then
        r=r-10
    ElseIf pawn_rank(LIGHT,f)=3 Then
        r=r-5
    EndIf

    eval_dkp=r
End Function
'=================================================================
'   MAIN.BAS
'   Tom Kerrigan's Simple Chess Program (TSCP)
'
'   Copyright 2019 Tom Kerrigan
'   port to MMBasic by ceptimus October 2019

' main is basically an infinite loop that either calls
' think() when it's the computer's turn to move or prompts
' the user for a command (and deciphers it).

init_arrays :main :End
Sub main
    Local computer_side,m,s$

    Cls
    Print
    Print "Tom Kerrigan's Simple Chess Program (TSCP)"
    Print "version 1.81c, 2/3/19"
    Print "Copyright 2019 Tom Kerrigan"
    Print "MMBasic conversion by ceptimus - "MMBasic_version$
    Print
    Print Chr$(34)+"help"+Chr$(34)+" displays a list of commands"
    Print
    init_hash
    init_board
    open_book
    gen
    computer_side=EMPTY
    max_time=2^24 ' this is about 4 hours, 39 minutes, 37 seconds
    'default search depth is 4 in Tom's compiled C version, but
    'that may be too slow on some MMBasic platforms so instead
    max_depth=2 ' default to 2. Can be changed by, say, user input: sd 4

    Do
        If side=computer_side Then ' computer's turn
            ' think about the move and make it
            think(1)
            If pv(0,0)=0 Then
                Print "(no legal moves)"
                computer_side=EMPTY
            Else
                Print "Computer's move: "move_str$(pv(0,0))
                m=makemove(pv(0,0)) 'm not used but required by function
                ply=0
                gen
                print_result
            EndIf
        Else ' user's turn
            ' get user input
            Input "tscp> ",s$
            If s$="on" Then
                computer_side=side
            ElseIf s$="off" Then
                computer_side=EMPTY
            ElseIf (Left$(s$,2)="st")And(Len(s$)>2) Then
                max_time=1000*Val(Mid$(s$,3))
                max_depth=32
            ElseIf (Left$(s$,2)="sd")And(Len(s$)>2) Then
                max_depth=Val(Mid$(s$,3))
                If max_depth<1 Then max_depth=1
                max_time=2^24
            ElseIf s$="undo" Then
                If hply<>0 Then
                    computer_side=EMPTY
                    takeback
                    ply=0
                    gen
                EndIf
            ElseIf s$="new" Then
                computer_side=EMPTY
                init_board
                gen
            ElseIf s$="d" Then
                print_board
            ElseIf s$="bench" Then
                computer_side=EMPTY
                bench
            ElseIf s$="bye" Then
                Print "Share and enjoy!"
                Exit Do
            'elseif s$="xboard" then 'not implemented in MMBasic
            '   xboard()
            ElseIf s$="help" Then
                Print "on - computer plays for the side to move"
                Print "off - computer stops playing"
                Print "st n - search for n seconds per move"
                Print "sd n - search n ply per move"
                Print "undo - takes back a move"
                Print "new - starts a new game"
                Print "d - display the board"
                Print "bench - run the built-in benchmark"
                Print "bye - exit the program"
                ' print"xboard - switch to XBoard mode"
                Print "Enter moves in coordinate notation, e.g., e2e4, e7e8Q"
            Else ' maybe the user entered a move?
                m=parse_move(s$)
                If m=-1 Then
                    Print "Illegal move."
                ElseIf makemove(gen_dat(m,0))=0 Then
                    Print "Illegal move."
                Else
                    ply=0
                    gen
                    print_result
                EndIf
            EndIf
        EndIf
    Loop
    close_book
End Sub

' parse the move s (in coordinate notation) and return the move's
' index in gen_dat, or -1 if the move is illegal
Function parse_move(vs$)
    Local from,too,i,a(4),s$

    parse_move=-1 'default to 'illegal move'
    s$=LCase$(vs$)

    ' make sure the string looks like a move
    If Len(s$)<4 Then Exit Function
    For i=0 To 3:a(i)=Asc(Mid$(s$,i+1)):Next i
    If (a(0)<Asc("a"))Or(a(0)>Asc("h"))Then Exit Function
    If (a(1)<Asc("1"))Or(a(1)>Asc("8"))Then Exit Function
    If (a(2)<Asc("a"))Or(a(2)>Asc("h"))Then Exit Function
    If (a(3)<Asc("1"))Or(a(3)>Asc("8"))Then Exit Function
    a(4)=Asc("q") 'default to queen promotion piece
    If Len(s$)>4 Then a(4)=Asc(Mid$(s$,5)) 'user entered promotion piece

    from=a(0)-Asc("a")
    from=from+8*(8-(a(1)-Asc("0")))
    too=a(2)-Asc("a")
    too=too+8*(8-(a(3)-Asc("0")))

    For i=0 To first_move(1)-1
        If (peek(VAR, gen_dat(i,0),0)=from)And(peek(VAR, gen_dat(i,0),1)=too)Then
            ' if the move is a promotion, handle the promotion piece;
            ' assume that the promotion moves occur consecutively in
            ' gen_dat.
            If (peek(VAR, gen_dat(i,0),3)And 32)<>0 Then
                Select Case a(4)
                    Case Asc("n")
                        parse_move=i:Exit For
                    Case Asc("b")
                        parse_move=i+1:Exit For
                    Case Asc("r")
                        parse_move=i+2:Exit For
                    Case Else 'assume it's a queen
                        parse_move=i+3:Exit For
                End Select
            EndIf
            parse_move=i:Exit For
        EndIf
    Next i
End Function

' move_str$ returns a string with move m in coordinate notation
Function move_str$(m)
    Local from=peek(VAR, m,0),too=peek(VAR, m,1)

    move_str$=Chr$((from And 7)+Asc("a"))
    move_str$=move_str$+Chr$(8-(from\8)+Asc("0"))
    move_str$=move_str$+Chr$((too And 7)+Asc("a"))
    move_str$=move_str$+Chr$(8-(too\8)+Asc("0"))

    If (peek(VAR, m,3)And 32)<>0 Then 'promotion
        Select Case peek(VAR, m,2)
            Case KNIGHT
                move_str$=move_str$+"N"
            Case BISHOP
                move_str$=move_str$+"B"
            Case ROOK
                move_str$=move_str$+"R"
            Case Else
                move_str$=move_str$+"Q"
        End Select
    EndIf
End Function

' print_board() prints the board
Sub print_board
    Local i

    Print :Print "8 ";
    For i=0 To 63
        Select Case hue(i)
            Case EMPTY
                Print " .";
            Case LIGHT
                Print " "Mid$(piece_char$,piece(i)+1,1);
            Case DARK
                Print " "Mid$(piece_char$,piece(i)+7,1);
        End Select
        If ((i+1)Mod 8=0)And(i<>63) Then Print :Print Str$(7-(i\8))" ";
    Next i
    Print :Print :Print "   a b c d e f g h":Print
End Sub

' print_result() checks to see if the game is over, and if so,
' prints the result.
Sub print_result
    Local i

    ' is there a legal move?
    For i=0 To first_move(1)-1
        If makemove(gen_dat(i,0))<>0 Then takeback:Exit For
    Next i
    If i=first_move(1)) Then
        If in_check(side)<>0 Then
            If side=LIGHT Then
                Print "0-1 {Black mates}"
            Else
                Print "1-0 {White mates}"
            EndIf
        Else
            Print "1/2-1/2 {Stalemate}"
        EndIf
    ElseIf reps()=2 Then
        Print "1/2-1/2 {Draw by repetition}"
    ElseIf fifty>=100 Then
        Print "1/2-1/2 {Draw by fifty move rule}"
    EndIf
End Sub

' bench: This is a little benchmark code that calculates how many
' nodes per second TSCP searches.
' It sets the position to move 17 of Bobby Fischer vs. J. Sherwin,
' New Jersey State Open Championship, 9/2/1957.
' Tom's original C version searches five ply three times.
' It calculates nodes per second from the best time.
' This MMBasic version runs too slowly to do that so instead just
' searches once for thirty seconds and uses the nodes per second
' value from that one search.
data_bench_color:
Data 6, 1, 1, 6, 6, 1, 1, 6
Data 1, 6, 6, 6, 6, 1, 1, 1
Data 6, 1, 6, 1, 1, 6, 1, 6
Data 6, 6, 6, 1, 6, 6, 0, 6
Data 6, 6, 1, 0, 6, 6, 6, 6
Data 6, 6, 0, 6, 6, 6, 0, 6
Data 0, 0, 0, 6, 6, 0, 0, 0
Data 0, 6, 0, 6, 0, 6, 0, 6

data_bench_piece:
Data 6, 3, 2, 6, 6, 3, 5, 6
Data 0, 6, 6, 6, 6, 0, 0, 0
Data 6, 0, 6, 4, 0, 6, 1, 6
Data 6, 6, 6, 1, 6, 6, 1, 6
Data 6, 6, 0, 0, 6, 6, 6, 6
Data 6, 6, 0, 6, 6, 6, 0, 6
Data 0, 0, 4, 6, 6, 0, 2, 0
Data 3, 6, 2, 6, 3, 6, 5, 6

Sub bench
    Local i,t
    Local float nps

    ' setting the position to a non-initial position confuses the opening
    ' book code.
    close_book

    Restore data_bench_color:For i=0 To 63:Read hue(i):Next i
    Restore data_bench_piece:For i=0 To 63:Read piece(i):Next i

    side=LIGHT
    xside=DARK
    castle=0
    ep=-1
    fifty=0
    ply=0
    hply=0
    set_hash
    print_board
    max_time=3e4 'thirty seconds
    max_depth=5
    think(1)
    t=Timer-start_time
    Print "Time: "Str$(t)" ms"
    Print :Print "Nodes: "Str$(nodes)

    If t=0 Then
        Print "(invalid)"
    Else
        nps=nodes
        nps=nps*1000.0/t
        ' Score: 1.000 = my Athlon XP 2000+
        Print "Nodes per second: "Str$(Int(nps+0.5));
        Print " (Score: "Str$(nps/243169.0,0,6)")"
    EndIf

    init_board
    max_time=2^24
    max_depth=2
    open_book
    gen
End Sub
'=================================================================
'   SEARCH.BAS
'   Tom Kerrigan's Simple Chess Program (TSCP)
'
'   Copyright 1997 Tom Kerrigan
'   port to MMBasic by ceptimus October 2019

' think() calls search() iteratively. Search statistics
' are printed depending on the value of output:
' 0 = no output
' 1 = normal output
Sub think(output)
    Local i,j,x

    ' try the opening book first
    pv(0,0)=book_move():If pv(0,0)<>-1 Then Exit Sub

    stop_search=0 'global flag gets set when time is up
    start_time=Timer
    stop_time=start_time+max_time

    ply=0
    nodes=0
    For i=0 To MAX_PLY-1
        For j=0 To MAX_PLY-1
            pv(i,j)=0
        Next j
     Next i
    For i=0 To 63
        For j=0 To 63
            history(i,j)=0
        Next j
    Next i

    If output=1 Then Print "ply      nodes  score  pv"
    For i=1 To max_depth
        follow_pv=1
        x=search(-10000,10000,i)
        If stop_search<>0 Then Exit For
        If output=1 Then
            Print Str$(i, 3)"  "Str$(nodes, 9)"  "Str$(x, 5)" ";
            For j=0 To pv_length(0)-1
                Print " "move_str$(pv(0,j));
            Next j
            Print
        EndIf
        If x>9000 Or x<-9000 Then i=max_depth
    Next i
    If stop_search<>0 Then
        Do
            If ply=0 Then Exit Do
            takeback()
        Loop
    EndIf
End Sub

' search() does just that, in negamax fashion
Function search(a,b,d)
    Local alpha=a,beta=b,depth=d 'pseudo pass by value
    Local i,j,x,c,f,from,too,exitF=0

    'default return value ready for if/when stop_search unwinds recursion
    search=alpha
    If stop_search<>0 Then Exit Function 'possible unwind from quiesce

    ' we're as deep as we want to be; call quiesce() to get
    ' a reasonable score and return it.
    If depth=0 Then search=quiesce(alpha,beta):Exit Function
    nodes=nodes+1

    ' do some housekeeping every 128 nodes
    If (nodes And 127)=0 Then
        If Timer>stop_time Then stop_search=1
    EndIf
    If stop_search<>0 Then Exit Function

    pv_length(ply)=ply

    ' if this isn't the root of the search tree (where we have
    ' to pick a move and can't simply return 0) then check to
    ' see if the position is a repeat. if so, we can assume that
    ' this line is a draw and return 0.
    If (ply<>0) Then
        If reps()<>0 Then search=0:Exit Function
    EndIf

    ' are we too deep?
    If (ply>=MAX_PLY-1)Or(hply>=HIST_STACK-1) Then
        search=eval_():Exit Function
    EndIf

    ' are we in check? if so, we want to search deeper
    c=in_check(side):If c<>0 Then depth=depth+1

    gen()
    If follow_pv<>0 Then sort_pv() 'are we following the PV?

    f=0

    ' loop through the moves
    For i=first_move(ply) To first_move(ply+1)-1
        sort(i)
        If makemove(gen_dat(i,0))<>0 Then
            f=1
            x=-search(-beta,-alpha,depth-1)
            If stop_search<>0 Then Exit For
            takeback()
            If x>alpha Then
                ' this move caused a cutoff, so increase the history
                ' value so it gets ordered high next time we can
                ' search it
                from=peek(VAR, gen_dat(i,0),0)
                too=peek(VAR, gen_dat(i,0),1)
                history(from,too)=history(from,too)+depth
                If x>=beta Then search=beta:exitF=1:Exit For
                alpha=x

                ' update the PV
                pv(ply,ply)=gen_dat(i,0)
                For j=ply+1 To pv_length(ply+1)-1
                    pv(ply,j)= pv(ply+1,j)
                Next j
                pv_length(ply)=pv_length(ply+1)
            EndIf
        EndIf
    Next i
    If (stop_search<>0)Or(exitF<>0) Then Exit Function

    ' no legal moves? then we're in checkmate or stalemate
    If f=0 Then
        search=0
        If c<>0 Then search=ply-10000
        Exit Function
    EndIf

    ' fifty move draw rule
    If fifty>=100 Then search=0:Exit Function

    search=alpha
End Function

' quiesce() is a recursive minimax search function with
' alpha-beta cutoffs. In other words, negamax. It basically
' only searches capture sequences and allows the evaluation
' function to cut the search off (and set alpha). The idea
' is to find a position where there isn't a lot going on
' so the static evaluation function will work.
Function quiesce(a,b)
    Local alpha=a,beta=b 'pseudo pass by value
    Local i,j,x,exitF=0

    'default return value ready for if/when stop_search unwinds recursion
    quiesce=alpha
    If stop_search<>0 Then Exit Function 'possible unwind from quiesce

    nodes=nodes+1

    ' do some housekeeping every 128 nodes
    If (nodes And 127)=0 Then
        If Timer>stop_time Then stop_search=1:Exit Function
    EndIf

    pv_length(ply)=ply

    ' are we too deep?
    If (ply>=MAX_PLY-1)Or(hply>=HIST_STACK-1) Then
        quiesce=eval_():Exit Function
    EndIf

    ' check with the evaluation function
    x=eval_():If x>=beta Then quiesce=beta:Exit Function

    If x>alpha Then alpha=x

    gen_caps()
    If follow_pv<>0 Then sort_pv() 'are we following the PV?

    ' loop through the moves
    For i=first_move(ply) To first_move(ply+1)-1
        sort(i)
        If makemove(gen_dat(i,0))<>0 Then
            x=-quiesce(-beta,-alpha)
            If stop_search<>0 Then Exit For
            takeback()
            If x>alpha Then
                If x>=beta Then quiesce=beta:exitF=1:Exit For
                alpha=x

                ' update the PV
                pv(ply,ply)=gen_dat(i,0)
                For j=ply+1 To pv_length(ply+1)-1
                    pv(ply,j)=pv(ply+1,j)
                Next j
                pv_length(ply)=pv_length(ply+1)
            EndIf
        EndIf
    Next i
    If (stop_search<>0)Or(exitF<>0) Then Exit Function
    quiesce=alpha
End Function

' reps() returns the number of times the current position
' has been repeated. It compares the current value of hash
' to previous values
Function reps()
    Local i

    reps=0
    For i=hply-fifty To hply-1
        If hist_dat(i,5)=hash Then reps=reps+1
    Next i
End Function

' sort_pv() is called when the search function is following
' the PV (Principal variation). It looks through the current
' ply's move list to see if the PV move is there. If so,
' it adds 10,000,000 to the move's score so it's played first
' by the search function. If not, follow_pv remains FALSE and
' search() stops calling sort_pv()
Sub sort_pv
    Local i

    follow_pv=0
    For i=first_move(ply) To first_move(ply+1)-1
        If gen_dat(i,0)=pv(0,ply) Then
            follow_pv=1
            gen_dat(i,1)=gen_dat(i,1)+1e7
            i=first_move(ply+1)-1
        EndIf
    Next i
End Sub

' sort() searches the current ply's move list from 'from'
' to the end to find the move with the highest score. Then it
' swaps that move and the 'from' move so the move with the
' highest score gets searched next, and hopefully produces
' a cutoff.
Sub sort(from)
    Local g(1)
    Local i,bs,bi

    bs=-1
    bi=from
    For i=from To first_move(ply+1)-1
        If gen_dat(i,1)>bs Then bs=gen_dat(i,1):bi=i
    Next i
    g(0)=gen_dat(from,0):g(1)=gen_dat(from,1)
    gen_dat(from,0)=gen_dat(bi,0):gen_dat(from,1)=gen_dat(bi,1)
    gen_dat(bi,0)=g(0):gen_dat(bi,1)=g(1)
End Sub
'=================================================================
