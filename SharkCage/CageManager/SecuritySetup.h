#pragma once

#include <memory>
#include <optional>

template<typename T>
auto local_free_deleter = [&](T resource) { ::LocalFree(resource); };

class SecuritySetup
{
public:
	~SecuritySetup()
	{
		if (security_descriptor)
		{
			::LocalFree(security_descriptor);
		}
	}

	std::optional<SECURITY_ATTRIBUTES> GetSecurityAttributes();

private:
	std::unique_ptr<PSID, decltype(local_free_deleter<PSID>)> CreateSID();
	std::optional<PACL> CreateACL(std::unique_ptr<PSID, decltype(local_free_deleter<PSID>)> group_sid);

	PSECURITY_DESCRIPTOR security_descriptor;
};
