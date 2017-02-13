AM_CFLAGS = -fPIC # -Wall -Wextra 
AM_CPPFLAGS = -fPIC # -Wall -Wextra 
AM_CXXFLAGS = -fPIC 

if DEBUG
AM_CPPFLAGS += -g3 -O3
AM_CXXFLAGS += -g3 -O3
endif

if PROFILE
AM_CPPFLAGS += -pg
AM_CXXFLAGS += -pg
endif

if MAC
AM_CPPFLAGS += -L/usr/X11R6/lib 
AM_CXXFLAGS += -L/usr/X11R6/lib
#AM_LIBS += -L/usr/X11R6/lib -lXss -lX11 -framework IOKit -framework Carbon
endif
