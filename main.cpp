#include <iostream>
#include <thread>
#include <ncurses.h>
using namespace std;


class object
{
public:
    string body = "***";
    int col;
    int row;    
};

class target
{
public:
    string body = "xxxxx";
    int row = 5;
    int col = 1;
};

class tank
{
    public:
        string body = "++++";
        int row = 15;
        int col = 1;
};


class projectile
{
    public:
        string body = "*";
        int row = 10;
        int col = 1;

};



int main()
{

    tank tnk;
    projectile pjt;
    target tgt;

    initscr();		
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);


    int ch;
    int tgt_col=1;
    int prj_row = tnk.row;
    int prj_col  = 1;
    bool shot=false;
    int max_col = 40;

    while (prj_row>=tgt.row)
    {
        tgt_col += 1;
        tgt_col = tgt_col % max_col;    
        move(tnk.row, tnk.col);
        printw("%s", tnk.body.c_str());

        move(tgt.row, tgt_col - 3);
        printw("    ");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        move(tgt.row, tgt_col);
        printw("%s", tgt.body.c_str());	

        refresh();

        if ((ch = getch())!=ERR){shot = true;};
        if(shot)
        {
            prj_row -= 1;
            move(prj_row, prj_col);
            printw("%s", pjt.body.c_str());
            refresh();	
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            move(prj_row, prj_col);
            prj_col += 2;
            printw(" ");
        };
    } 
    
  
	refresh();			
	getch();			
	endwin();			
	return 0;

}