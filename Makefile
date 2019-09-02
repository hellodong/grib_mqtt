BUILD_DIR:=build
SOURCE_DIR:=source
CC:=gcc
CFLAGS:=-Wall -g
LDFLAGS:=-ljson-c -lpthread -lmosquitto
BIN:=mqtt_grib

OBJS :=$(BUILD_DIR)/main.o
OBJS +=$(BUILD_DIR)/cloud.o
OBJS +=$(BUILD_DIR)/grib_proto.o
OBJS +=$(BUILD_DIR)/util.o

all:$(BUILD_DIR) $(BIN)

$(BUILD_DIR):
	mkdir $@
$(BIN):$(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o:$(SOURCE_DIR)/%.c
	$(CC) $< $(CFLAGS) -c -o $@

#$(BUILD_DIR)/main.o:$(SOURCE_DIR)/main.c
#	$(CC) $< $(CFLAGS) -c -o $@

#$(BUILD_DIR)/cloud.o:$(SOURCE_DIR)/cloud.c
#	$(CC) $< $(CFLAGS) -c -o $@

#$(BUILD_DIR)/util.o:$(SOURCE_DIR)/util.c
#	$(CC) $< $(CFLAGS) -c -o $@

.PHONY:clean

clean:
	rm $(BUILD_DIR) $(BIN) -rf

