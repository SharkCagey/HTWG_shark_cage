#pragma once

#include <memory>
#include <optional>
#include <string>

template<typename T>
auto local_free_deleter = [&](T *resource) { ::LocalFree(resource); };

typedef void Sid;

template<typename D>
using SidPointer = std::unique_ptr<Sid, D>;

class SecuritySetup
{
public:
	SecuritySetup() {};

	// these need to be customized if needed so security_descriptor also gets copied
	// (otherwise there could be use-after-frees in the destructor)
	SecuritySetup(const SecuritySetup &) = delete;
	SecuritySetup& operator= (const SecuritySetup &) = delete;

	~SecuritySetup()
	{
		if (security_descriptor)
		{
			::LocalFree(security_descriptor);
			security_descriptor = nullptr;
		}
	}

	std::optional<SECURITY_ATTRIBUTES> GetSecurityAttributes(const std::wstring &group_name);

private:
	SidPointer<decltype(local_free_deleter<Sid>)> CreateSID(const std::wstring &group_name);
	std::optional<PACL> CreateACL(SidPointer<decltype(local_free_deleter<Sid>)> group_sid);

	PSECURITY_DESCRIPTOR security_descriptor;
};
