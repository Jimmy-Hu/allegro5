/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Example program showing how to use Allegro as a pure
 *      sound library under Windows.
 *
 *      By Eric Botcazou.
 * 
 *      Original idea and improvements by Javier Gonzalez.
 *
 *      See readme.txt for copyright information.
 */

#include "allegro.h"
#include "winalleg.h"

#ifndef SCAN_DEPEND
   #include <commdlg.h>
#endif

#include "dibsound.rh" 


HINSTANCE hInst = NULL;
PALETTE pal;
BITMAP *bmp = NULL;



BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
   switch(Message) {

      case WM_INITDIALOG:
         return TRUE;

      case WM_COMMAND:
         switch(LOWORD(wParam)) {

            case IDOK:
               EndDialog(hwnd, IDOK);
            return TRUE;

            case IDCANCEL:
               EndDialog(hwnd, IDCANCEL);
            return TRUE;
         }
         break;
   }

   return FALSE;
}



int OpenNewSample(SAMPLE **sample, HWND hwnd)
{
   OPENFILENAME openfilename;
   SAMPLE *new_sample = NULL;
   char filename[512];

   memset(&openfilename, 0, sizeof(OPENFILENAME));
   openfilename.lStructSize = sizeof(OPENFILENAME);
   openfilename.hwndOwner = hwnd;
   openfilename.lpstrFilter = "Sounds (*.wav;*.voc)\0*.wav;*.voc\0";
   openfilename.lpstrFile = filename;
   openfilename.nMaxFile = sizeof(filename);
   openfilename.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

   filename[0] = 0;

   if (GetOpenFileName(&openfilename)) {
      new_sample = load_sample(openfilename.lpstrFile);

      if (!new_sample) {
         MessageBox(hwnd, "This is not a standard sound file.", "Error!",
                    MB_ICONERROR | MB_OK);
         return -1;
      }

      /* make room for next samples */
      if (*sample)
         destroy_sample(*sample);

      *sample = new_sample;
      return 0;
   }

   return -1;
}



LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
   static SAMPLE *sample = NULL;
   HDC hdc;
   RECT updaterect;
   PAINTSTRUCT ps;

   switch(Message) {

      case WM_COMMAND:
         switch(LOWORD(wParam)) {

            case CMD_FILE_OPEN:
               OpenNewSample(&sample, hwnd);
               break;

            case CMD_FILE_PLAY:
               if (!sample) {
                  if (OpenNewSample(&sample, hwnd) != 0)
                     break;
               }

               /* play it !! */
               play_sample(sample, 128, 128, 1000, FALSE);
               break;

            case CMD_FILE_EXIT:
               PostMessage(hwnd, WM_CLOSE, 0, 0);
               break;

            case CMD_HELP_ABOUT:
               DialogBox(hInst, "ABOUTDLG", hwnd, AboutDlgProc);
               break;
         }
         return 0;

      case WM_PAINT:
         if (GetUpdateRect(hwnd, &updaterect, FALSE)) {
            hdc = BeginPaint(hwnd, &ps);
            set_palette_to_hdc(hdc, pal);
            draw_to_hdc(hdc, bmp, 0, 0);
            EndPaint(hwnd, &ps);
         }
         return 0;

      case WM_CLOSE:
         /* call remove_sound() before destroying the window */
         destroy_sample(sample);
         remove_sound();
         DestroyWindow(hwnd);
         return 0;

      case WM_DESTROY:
         PostQuitMessage(0);
         return 0;
   }

   return DefWindowProc(hwnd, Message, wParam, lParam);
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   static char szAppName[] = "Sound Player";
   WNDCLASS wndClass;
   HWND hwnd;
   HACCEL haccel;
   MSG msg;

   hInst = hInstance;

   if (!hPrevInstance) {
      wndClass.style         = CS_HREDRAW | CS_VREDRAW;
      wndClass.lpfnWndProc   = WndProc;
      wndClass.cbClsExtra    = 0;
      wndClass.cbWndExtra    = 0;
      wndClass.hInstance     = hInst;
      wndClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
      wndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
      wndClass.lpszMenuName  = "MYMENU";
      wndClass.lpszClassName = szAppName;

      RegisterClass(&wndClass);
   }  

   hwnd = CreateWindow(szAppName, szAppName,
                       WS_OVERLAPPEDWINDOW,
                       CW_USEDEFAULT, CW_USEDEFAULT,
                       320, 240,
                       NULL, NULL,
                       hInst, NULL);

   if (!hwnd) {
      MessageBox(0, "Window Creation Failed.", "Error!",
                 MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
      return 0;
   }

   /* register our window BEFORE calling allegro_init() */
   win_set_window(hwnd);

   /* initialize the library */
   allegro_init();
   install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);

   set_color_conversion(COLORCONV_NONE);

   /* load some 8 bit bitmap */
   bmp = load_bitmap("..\\..\\examples\\allegro.pcx", pal);
   if (!bmp) {
      MessageBox(hwnd, "Can't load ..\\..\\examples\\allegro.pcx", "Error!",
                 MB_ICONERROR | MB_OK);
      return 0;
   }

   /* display the window */
   haccel = LoadAccelerators(hInst, "MYACCEL");
   ShowWindow(hwnd, nCmdShow);
   UpdateWindow(hwnd);

   /* process messages */
   while(GetMessage(&msg, NULL, 0, 0)) {
      if (!TranslateAccelerator(hwnd, haccel, &msg)) {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }

   return msg.wParam;
}
