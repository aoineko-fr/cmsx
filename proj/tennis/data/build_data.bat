@echo -----------------------------------------------------------------------------
@echo  CONVERT SC2 IMAGES

..\..\..\tools\CMSXimg\CMSXimg.exe logo.png    -out ..\data_logo.h    -mode gm2   -def	-name g_DataLogo

..\..\..\tools\CMSXimg\CMSXimg.exe court.png   -out ..\data_court.h   -mode gm2   -def	-name g_DataCourt

@echo.
@echo -----------------------------------------------------------------------------
@echo  CONVERT SPRITES

..\..\..\tools\CMSXimg\CMSXimg.exe logo_ball.png -out ..\data_logo_ball.h -mode sprt -name g_DataLogoBall -pos 164 41 -size 16 16 -num 2 2 ^
	-l i16 0 0 1 1 0xDED087 ^
	-l i16 0 0 1 1 0xB95E51

..\..\..\tools\CMSXimg\CMSXimg.exe players.png -out ..\data_racket.h  -mode bmp  -name g_DataRacket  -pos 0 192 -size 8 8 -num 16 1 -bpc 1 -trans 0xFF00FF

..\..\..\tools\CMSXimg\CMSXimg.exe players.png -out ..\data_ball.h    -mode sprt -name g_DataBall    -pos 144 0 -size 8 8 -gap 8 0 -num 5 1 ^
	-l i8 0 0 1 1 0x010101 ^
	-l i8 0 0 1 1 0x010101 0x706944 ^
	-l i8 0 0 1 1 0xDED087 0x706944

..\..\..\tools\CMSXimg\CMSXimg.exe players.png -out ..\data_score.h   -mode sprt -name g_DataScore   -pos 0 200 -size 16 16 -num 2 3 ^
	-l i16 0 0 1 1 0x010101 ^
	-l i16 0 0 1 1 0xFFFFFF

..\..\..\tools\CMSXimg\CMSXimg.exe players.png -out ..\data_player1.h -mode sprt -name g_DataPlayer1 -pos 0 0 -size 16 24 -num 9 3 ^
	-l i16 0  0 1 1 0x010101 ^
	-l i16 0  0 1 1 0x010101 0x403B78 0x7F7F7F 0x7F453F ^
	-l i16 0  8 1 1 0x8076F1 0x403B78 ^
	-l i16 0  0 1 1 0xFFFFFF 0x7F7F7F ^
	-l i16 0  0 1 1 0xFF897D 0x7F453F ^
	-l i8  0 16 2 1 0x010101 ^
	-l i8  0 16 2 1 0x010101 0x403B78 0x7F7F7F 0x7F453F ^
	-l i8  0 16 2 1 0xFFFFFF 0x7F7F7F ^
	-l i8  0 16 2 1 0xFF897D 0x7F453F

..\..\..\tools\CMSXimg\CMSXimg.exe players.png -out ..\data_player2.h -mode sprt -name g_DataPlayer2 -pos 0 96 -size 16 24 -num 9 3 ^
	-l i8  0 0 2 1 0x010101 ^
	-l i8  0 0 2 1 0x010101 0x1F5C24 0x7F7F7F 0x7F453F ^
	-l i8  0 0 2 1 0xFFFFFF 0x7F7F7F ^
	-l i8  0 0 2 1 0xFF897D 0x7F453F ^
	-l i16 0 8 1 1 0x010101 ^
	-l i16 0 8 1 1 0x010101 0x1F5C24 0x7F7F7F 0x7F453F ^
	-l i16 0 0 1 1 0x3EB849 0x1F5C24 ^
	-l i16 0 8 1 1 0xFFFFFF 0x7F7F7F ^
	-l i16 0 8 1 1 0xFF897D 0x7F453F

@echo.
@echo -----------------------------------------------------------------------------
@echo  CONVERT SFX

..\..\..\tools\CMSXbin\CMSXbin.exe ayfx_bank.afb -o ..\data_sfx.h -ad

@echo.
@echo -----------------------------------------------------------------------------
@echo  CONVERT MUSIC

..\..\..\tools\CMSXbin\CMSXbin.exe intro.pt3 -o ..\data_music.h -skip 0 100 -ad


pause