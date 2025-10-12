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
    End If
End Sub

' (file continues...)