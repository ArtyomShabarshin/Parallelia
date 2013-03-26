#ifndef AUTOMATICGUARD_H
#define AUTOMATICGUARD_H


namespace Parallelia
{

	namespace ParalleliaCore
	{
		template<typename T>
		class AutomaticGuard
		{
			typedef std::function<T()> F1;
			typedef std::function<void(void)> F2;
		public:
			AutomaticGuard(F1 init, F2 finish) : m_finish(finish)
												,m_isdropped(false)
			{ m_result = init();}

			virtual ~AutomaticGuard()
			{ if(!m_isdropped) m_finish();}
			T& Result() const  { return m_result; }
			void Drop() { m_isdropped = true; }
		private:
			AutomaticGuard(const AutomaticGuard&);
			AutomaticGuard& operator=(const AutomaticGuard&);
			T m_result;
		private:
			F2 m_finish;
			bool m_isdropped;
		};


	}


}


#endif