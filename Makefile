CPPFLAGS = -I$(HOME)/src/boost
CXXFLAGS = -O3
LDLIBS   = -lprotobuf

PROTO    = $(wildcard *.proto)
PROTOSRC = $(addsuffix .pb.cc, $(basename $(PROTO)))
PROTOH   = $(addsuffix .pb.h, $(basename $(PROTO)))
SRC      = $(wildcard *.cpp) $(PROTOSRC)
OBJ      = $(addsuffix .o, $(basename $(SRC)))

all: ProtoBench

ProtoBench: $(OBJ)
	$(LINK.cc) -o $@ $(OBJ) $(LDLIBS)

clean:
	$(RM) $(RMFLAGS) $(OBJ) $(PROTOSRC) $(PROTOH)

%.pb.cc %.pb.h: %.proto
	protoc --cpp_out=. $<

ProtoBench.o: $(PROTOH)
