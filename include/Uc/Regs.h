#pragma once

#include <unicorn/unicorn.h>

namespace Uc {
	struct Regs {
		Regs(uc_engine* uc);

		template<typename TStorage, typename TReg>
		inline TStorage Read(TReg regType)
		{
			TStorage stg{};
			uc_reg_read(mUc, regType, &stg);
			return stg;
		}

		template<typename TReg, typename TRegType>
		inline void Write(TRegType regType, TReg val)
		{
			uc_reg_write(mUc, regType, &val);
		}

		uc_engine* mUc;
	};
}