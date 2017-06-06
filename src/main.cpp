#include <QApplication>
#include "include/MyoExample.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MyoExample foo;
    foo.show();
    return app.exec();
}
