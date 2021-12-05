# TSCP
4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1 
8/pppppppp/8/8/8/8/PPPPPPPP/8 w - - 0 1 

For variant battle:
In board.c lines 
99 return FALSE;

401-403
  //color[from] = EMPTY;
		//piece[from] = EMPTY;
		piece[to] = piece[to] + 1;

436, 437:
	color[(int)m.from] = side;
	piece[(int)m.to] = piece[(int)m.from] + 1;

	In main.c line 414
	   printf("feature variants=\"normal,battle\"\n");
	   
	   line 438:
	   if (strcmp(variant,"normal") || strcmp(variant,"battle")) {
	   
In eval.c
line 27 //original 	90, 300, 300, 500, 900, 0
900, 850, 250, 60, -150, -2700   




	   
	   
	   Erster Schritt
	   g++ -c fen.c book.c search.c eval.c board.c main.c data.c 
	   
	   Zweiter Schritt
	   g++ -o tscp.exe main.o fen.o book.o search.o eval.o board.o data.o