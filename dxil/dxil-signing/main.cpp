#include <iostream>
#include <windows.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include "CLI11.hpp"
#include "dxcapi.h"

#pragma comment(lib, "d3dcompiler")

using Microsoft::WRL::ComPtr;

struct DxilMinimalHeader
{
	UINT32 four_cc;
	UINT32 hash_digest[4];
};

inline bool is_dxil_signed(void* buffer)
{
	DxilMinimalHeader* header = reinterpret_cast<DxilMinimalHeader*>(buffer);
	bool has_digest = false;
	has_digest |= header->hash_digest[0] != 0x0;
	has_digest |= header->hash_digest[1] != 0x0;
	has_digest |= header->hash_digest[2] != 0x0;
	has_digest |= header->hash_digest[3] != 0x0;
	return has_digest;
}

int main(int argc, const char* argv[])
{
	CLI::App app{ "DXIL Signing Utility" };

	std::string filename = "";
	app.add_option("filename", filename, "File to change in-place")->required();

	CLI11_PARSE(app, argc, argv);

	std::cout << "Loading input file: " << filename << std::endl;

	FILE* f = fopen(filename.c_str(), "r+b");
	if (f == nullptr)
	{
		std::cout << "Failed to open input file" << std::endl;
		exit(1);
	}

	fseek(f, 0, SEEK_END);
	size_t input_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	std::vector<uint8_t> dxil_data;
	dxil_data.resize(input_size);

	size_t bytes_read = fread(dxil_data.data(), 1, dxil_data.size(), f);

	if (bytes_read != dxil_data.size() || bytes_read < sizeof(DxilMinimalHeader))
	{
		std::cout << "Failed to read input file" << std::endl;
		exit(1);
	}

	// Ensure the binary isn't already signed
	if (is_dxil_signed(dxil_data.data()))
	{
		std::cout << "Input file is already signed" << std::endl;
		exit(1);
	}

	HMODULE dxil_module = ::LoadLibrary(L"C:\\Program Files (x86)\\Windows Kits\\10\\Redist\\D3D\\x64\\dxil.dll");
	if (dxil_module == nullptr)
	{
		std::cout << "Failed to load dxil.dll" << std::endl;
		exit(1);
	}

	DxcCreateInstanceProc dxil_create_func = (DxcCreateInstanceProc)GetProcAddress(dxil_module, "DxcCreateInstance");
	if (dxil_create_func == nullptr)
	{
		std::cout << "Failed to get dxil create proc" << std::endl;
		exit(1);
	}

	ComPtr<ID3DBlob> blob;
	D3DCreateBlob(dxil_data.size(), &blob);

	// Copy into the blob.
	memcpy(blob->GetBufferPointer(), dxil_data.data(), dxil_data.size());

	ComPtr<IDxcValidator> validator;
	if (FAILED(dxil_create_func(CLSID_DxcValidator, __uuidof(IDxcValidator), (void**)&validator)))
	{
		std::cout << "Failed to create validator instance" << std::endl;
		exit(1);
	}

	// Query validation version info
	{
		ComPtr<IDxcVersionInfo> version_info;
		if (FAILED(validator->QueryInterface(__uuidof(IDxcVersionInfo), (void**)&version_info)))
		{
			std::cout << "Failed to query version info interface" << std::endl;
			exit(1);
		}

		UINT32 major = 0;
		UINT32 minor = 0;
		version_info->GetVersion(&major, &minor);
		std::cout << "Validator version: " << major << "." << minor << std::endl;
	}

	ComPtr<IDxcOperationResult> result;
	if (FAILED(validator->Validate((IDxcBlob*)blob.Get(), DxcValidatorFlags_InPlaceEdit /* avoid extra copy owned by dxil.dll */, &result)))
	{
		std::cout << "Failed to validate dxil container" << std::endl;
		exit(1);
	}

	HRESULT validateStatus;
	if (FAILED(result->GetStatus(&validateStatus)))
	{
		std::cout << "Failed to get dxil validate status" << std::endl;
		exit(1);
	}

	if (FAILED(validateStatus))
	{
		std::cout << "The dxil container failed validation" << std::endl;

		ComPtr<ID3DBlob> error;
		result->GetErrorBuffer((IDxcBlobEncoding**)error.GetAddressOf());

		std::string errorString((const char*)error->GetBufferPointer(), error->GetBufferSize());

		std::cout << "Error: " << std::endl << errorString << std::endl;

		exit(2);
	}
	result.Reset();

	validator = nullptr;

	// Ensure the binary is now signed
	if (!is_dxil_signed(blob->GetBufferPointer()))
	{
		std::cout << "Signing failed!" << std::endl;
		exit(1);
	}

	std::cout << "Saving output file: " << filename << std::endl;

	fseek(f, 0, SEEK_SET);
	size_t bytes_written = fwrite(blob->GetBufferPointer(), 1, blob->GetBufferSize(), f);

	if (bytes_written != dxil_data.size())
	{
		std::cout << "Failed to write output file" << std::endl;
		exit(1);
	}

	blob->Release();

	fclose(f);

	::FreeLibrary(dxil_module);
}