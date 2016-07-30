#include <application.h>

using namespace std;

std::string current_file;
std::vector<object_t> ObjectStack;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
bool quit = false;
bool sim = false;
void ctrl_c_handler(int s)
{
    printf("Bye!\n");
    quit = true; //Shutdown clean!
}
int main( int argc, char* argv[] )
{
  current_activity = "Main";
  #ifdef DEBUG
    sim = true;
  #endif
	//Start up SDL and create window
	if( !Render_Init() )
	{
		printf("Failed to initialize!\n");
	}
	else
	{
    #ifdef NDEBUG
      SDL_ShowCursor(0);
    #endif
		Config_Init();
		if (Serial_Init() < 0)
		{
			Render_Close();
			exit(1);
		}
		SDL_Event e;
		//While application is running
    bool MouseDown = false;
    point_t LastMouseMovePos;

    signal (SIGINT,ctrl_c_handler);
		while( !quit )
		{
  			while( SDL_PollEvent( &e ) != 0 )
  			{
  				//User requests quit
  				if( e.type == SDL_QUIT )
  				{
  					quit = true;
  				}
  				if (e.type == SDL_KEYDOWN)
  				{
  					switch( e.key.keysym.sym )
        		{
        			case SDLK_ESCAPE: quit = true; break;
  						case SDLK_SPACE: Config_Init(); break;
        		}
  				}
          if (e.type == SDL_MOUSEMOTION )
          {
            if (MouseDown == true)
            {
              int x = 0, y = 0;
              SDL_GetMouseState(&x, &y);
              if (current_activity == "FileOpen")
              {
                //printf("Scroll Inc: %0.6f\n", LastMouseMovePos.y - y);
                if (file_open_scroll_offset > 0)
                {
                  file_open_scroll_offset = 0;
                }
                file_open_scroll_offset += (y - LastMouseMovePos.y);

              }
              LastMouseMovePos.x = (float)x;
              LastMouseMovePos.y = (float)y;
              //printf("Move: %d,%d\n", x, y);
            }

          }
          if( e.type == SDL_MOUSEBUTTONUP )
          {
            //printf("Button Release!\n");
            if( e.button.button == SDL_BUTTON_LEFT )
            {
              MouseDown = false;
            }
          }
          if( e.type == SDL_MOUSEBUTTONDOWN )
          {
            //If the left mouse button was pressed
            if( e.button.button == SDL_BUTTON_LEFT )
            {
                int x = 0, y = 0;
                SDL_GetMouseState(&x, &y);
                LastMouseMovePos.x = (float)x;
                LastMouseMovePos.y = (float)y;
                MouseDown = true;
                //int mx = e.tfinger.x * SCREEN_WIDTH;
                //int my = e.tfinger.y * SCREEN_HEIGHT;

                //Doesnt work on pi sdl2-2.0.3 but works with sdl2-2.0.4 but with a wierd number set, like 100 - 4020 on x and y
                int mx, my;
                if (sim)
                {
                  mx = e.button.x;
                  my = e.button.y;
                }
                else
                {
                  mx = map_range(e.button.x, 10, 4020, 0, SCREEN_WIDTH);
                  my = map_range(e.button.y, 10, 4020, 0, SCREEN_HEIGHT);
                }

                /*if (mx > 800 && my > 4000)
                {
                  printf("Powering down!");
                  Render_Close();
                  system("poweroff");
                }*/
                //Doesnt work on PI
                //int mx, my;
                //SDL_GetMouseState(&mx, &my);

                //printf("Clicked at %d,%d\n", mx , my);
                string clicked = "";
                if (current_activity == "FileOpen")
                {
                  for(int x = 0; x < file_tiles.size(); x++)
                  {
                    if( ( mx > file_tiles[x].position.x ) && ( mx < file_tiles[x].position.x + file_tiles[x].size.w  ) &&
                        ( my > file_tiles[x].position.y ) && ( my < file_tiles[x].position.y + file_tiles[x].size.h ) )
                    {
                      printf("Clicked File -> %s\n", file_tiles[x].tagname.c_str());
                      current_file = file_tiles[x].tagname;
                      current_activity = "Main";
                    }
                  }
                }
                for(int x = 0; x < ObjectStack.size(); x++)
                {
                  if (ObjectStack[x].type == INPUT && ObjectStack[x].visable == true && ObjectStack[x].activity == current_activity )
                  //if (ObjectStack[x].visable == true)
                  {
                    if( ( mx > ObjectStack[x].position.x ) && ( mx < ObjectStack[x].position.x + ObjectStack[x].size.w  ) &&
                        ( my > ObjectStack[x].position.y ) && ( my < ObjectStack[x].position.y + ObjectStack[x].size.h ) )
                    {
                      //printf("Clicked Tag -> %s\n", ObjectStack[x].tagname.c_str());
                      clicked = ObjectStack[x].tagname;
                    }
                    //printf("Tagname - > %s\n", ObjectStack[x].tagname.c_str());
                    //printf("\tposition.x = %d\n", ObjectStack[x].position.x);
                    //printf("\tposition.y = %d\n", ObjectStack[x].position.y);
                    //printf("\tsize.w = %d\n", ObjectStack[x].size.w);
                    //printf("\tsize.h = %d\n", ObjectStack[x].size.h);
                  }
                }
                for(int x = 0; x < ObjectStack.size(); x++)
                {
                  if (ObjectStack[x].tagname == clicked)
                  {
                    if (ObjectStack[x].tagname == "PowerOff")
                    {
                      #ifdef NDEBUG
                        system("poweroff");
                      #else
                        printf("Powering off!\n");
                        quit = true;
                      #endif

                    }
                    if (ObjectStack[x].tagname == "FileOpen")
                    {
                      current_activity = "FileOpen";
                    }
                    if (ObjectStack[x].tagname == "BackButton")
                    {
                      current_activity = "Main";
                    }
                    if (ObjectStack[x].tagname == "SetOrigin")
                    {
                      ////CNC_SetOrigin();
                      Serial_WriteString("G92 X0 Y0");
                    }
                    else if (ObjectStack[x].tagname == "Stop")
                    {
                      //CNC_Stop();
                      int ctrl_x = 24;
                      Serial_WriteString(to_string(ctrl_x)); //not tested
                      Sender_Stop();
                    }
                    else if (ObjectStack[x].tagname == "Start")
                    {
                      //CNC_Start();
                      Serial_WriteString("~");
                      MachineState = "Run";
                    }
                    else if (ObjectStack[x].tagname == "Hold")
                    {
                      //CNC_Hold();
                      Serial_WriteString("!");
                      MachineState = "Hold";
                    }
                    else
                    {
                      //printf("Clicked Tag -> %s at index %d\n", ObjectStack[x].tagname.c_str(), x);
                      while(1) //Should add breakout timer so we cant hang!
                      {
                        //printf("Waiting for release!\n");
                        SDL_PollEvent( &e );
                        if( e.type == SDL_MOUSEBUTTONUP )
                        {
                          //printf("Button Release!\n");
                          MouseDown = false;
                          break;
                        }
                        if (ObjectStack[x].tagname == "JogXPlus")
                        {
                          //CNC_JogXPlus();
                          Serial_WriteString("G91G0X0.001");
                          usleep(6000);
                        }
                        if (ObjectStack[x].tagname == "JogXMinus")
                        {
                          //CNC_JogXMinus();
                          Serial_WriteString("G91G0X-0.001");
                          usleep(6000);
                        }
                        if (ObjectStack[x].tagname == "JogYPlus")
                        {
                          Serial_WriteString("G91G0Y0.001");
                          usleep(6000);
                        }
                        if (ObjectStack[x].tagname == "JogYMinus")
                        {
                          Serial_WriteString("G91G0Y-0.001");
                          usleep(6000);
                        }
                        if (ObjectStack[x].tagname == "JogZPlus")
                        {
                          Serial_WriteString("G91G0Z0.001");
                          usleep(6000);
                        }
                        if (ObjectStack[x].tagname == "JogZMinus")
                        {
                          Serial_WriteString("G91G0Z-0.001");
                          usleep(6000);
                        }
                      }
                    }

                    //Config_RunScript(ObjectStack[x].tagname);
                    break; //Only run one function!
                  }
                }

              }
            }
  			}
        Sender_Tick();
        Serial_Read();
        Render_RenderStack();
        SDL_Delay(100);
		}
	}

	//Free resources and close SDL
	Render_Close();
	Serial_Close();
	return 0;
}
