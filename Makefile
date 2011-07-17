
BUILDDIR := Debug
TARGET := $(BUILDDIR)/mare
FILES := $(shell find src -name \*.cpp)
OBJECTS := $(foreach file,$(patsubst %.cpp,%.o,$(FILES)),$(BUILDDIR)/$(file))

.PHONY: all clean prebuild

all: $(TARGET)

prebuild:
	@mkdir -p $(dir $(OBJECTS))

$(BUILDDIR)/%.o: %.cpp | prebuild
	@echo "$<"
	@$(CXX) -MMD -Wall -g -o $@ -c $<

$(TARGET): $(OBJECTS) | prebuild
	@echo Linking $(notdir $@)...
	@$(CXX) -o $@ $(OBJECTS)

clean:
	-@$(RM) $(OBJECTS) $(patsubst %.o,%.d,$(OBJECTS)) $(TARGET)

-include $(patsubst %.o,%.d,$(OBJECTS))

