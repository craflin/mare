
BUILDDIR := Debug
TARGET := $(BUILDDIR)/mare
FILES := $(shell find src -name \*.cpp)
OBJECTS := $(foreach file,$(patsubst %.cpp,%.o,$(FILES)),$(BUILDDIR)/$(file))
DIRS := $(dir $(OBJECTS))

.PHONY: all clean prebuild

all: $(TARGET)

prebuild:
	@mkdir -p $(DIRS)

$(BUILDDIR)/%.o: %.cpp | prebuild
	@echo "$<"
	@$(CXX) -MMD -o $@ -c $<

$(TARGET): $(OBJECTS) | prebuild
	@echo Linking $(notdir $@)...
	@$(CXX) -o $@ $(OBJECTS)

clean:
	-@$(RM) $(OBJECTS) $(TARGET)

-include $(patsubst %.o,%.d,$(OBJECTS))

