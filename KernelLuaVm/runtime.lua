R"(
    function malloc(len)
        return nt.ExAllocatePoolWithTag(0, len, 0)
    end
    function free(ptr)
        return nt.ExFreePoolWithTag(ptr, 0)
    end

    function tmp(bytes)
        local meta = {
            __gc = function(self)
                if self.pointer ~= 0 then
                    free(self.pointer)
                end
            end
        }
        local self = { 
            pointer = bytes > 0 and malloc(bytes) or 0,
            size = bytes,

            ref = function(self)
                return self.pointer
            end,

            get = function(self, i)
                i = i or 0
                if self.size >= (8+i)  then
                    return read8(self.pointer + i)
                elseif self.size >= (4+i)  then
                    return read4(self.pointer + i)
                elseif self.size >= (2+i)  then
                    return read2(self.pointer + i)
                elseif self.size >= (1+i)  then
                    return read1(self.pointer + i)
                else
                    return nil
                end
            end,
        
            set = function(self, v, i)
                i = i or 0
                if self.size >= (8+i)  then
                    return write8(self.pointer + i, v)
                elseif self.size >= (4+i)  then
                    return write4(self.pointer + i, v)
                elseif self.size >= (2+i)  then
                    return write2(self.pointer + i, v)
                elseif self.size >= (1+i)  then
                    return write1(self.pointer + i, v)
                else
                    return nil
                end
            end
        }
        setmetatable(self, meta)
        return self
    end

    function ansi_string(str)
        local AnsiString = tmp(0x10 + #str)
        write4(AnsiString:ref(), 0x10001 * #str)
        write8(AnsiString:ref() + 8, AnsiString:ref() + 0x10)
        memcpy(AnsiString:ref() + 0x10, addressof(str), #str)
        return AnsiString
    end
    
    function unicode_string(str)
        local UnicodeString = tmp(0x10 + (#str*2))
        write4(UnicodeString:ref(), 0x10001 * (#str*2))
        write8(UnicodeString:ref() + 8, UnicodeString:ref() + 0x10)
        
        local Dst = UnicodeString:ref() + 0x10
        local Src = addressof(str)
        for i=0,#str do
          write2(Dst + i * 2, read1(Src + i))
        end
        
        return UnicodeString
    end
)"



