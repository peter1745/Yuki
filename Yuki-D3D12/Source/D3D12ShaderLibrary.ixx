module;

#include "D3D12Common.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>

export module D3D12:ShaderLibrary;

import Aura;
import Yuki.Rendering;
import Yuki.Windows;

import :RHIHandles;

namespace Yuki {

	ShaderLibrary ShaderLibrary::Create(RHIContext context)
	{
		auto* impl = new Impl();

		// Setup DirectX Shader Compiler
		CheckHR(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&impl->DxcUtils)));
		CheckHR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&impl->Compiler)));

		return { impl };
	}

	Shader ShaderLibrary::Compile(const std::filesystem::path& filepath) const
	{
		std::ifstream stream(filepath);
		std::stringstream ss;
		ss << stream.rdbuf();
		stream.close();

		std::string source = ss.str();

		IDxcBlobEncoding* sourceBlob;
		CheckHR(m_Impl->DxcUtils->CreateBlob(source.c_str(), source.size(), CP_UTF8, &sourceBlob));

		auto args = std::array
		{
			DXC_ARG_DEBUG,
			DXC_ARG_ENABLE_STRICTNESS,
			DXC_ARG_IEEE_STRICTNESS,
			DXC_ARG_WARNINGS_ARE_ERRORS,
		};

		IDxcCompilerArgs* compilerArgs;
		CheckHR(m_Impl->DxcUtils->BuildArguments(
			filepath.filename().c_str(),
			L"VSMain",
			L"vs_6_7",
			args.data(),
			static_cast<UINT32>(args.size()),
			nullptr,
			0,
			&compilerArgs
		));

		DxcBuffer sourceBuffer =
		{
			.Ptr = sourceBlob->GetBufferPointer(),
			.Size = sourceBlob->GetBufferSize(),
			.Encoding = 0
		};

		IDxcResult* compileResult;
		CheckHR(m_Impl->Compiler->Compile(&sourceBuffer, compilerArgs->GetArguments(), compilerArgs->GetCount(), nullptr, IID_PPV_ARGS(&compileResult)));

		IDxcBlobUtf8* errorMessage;
		compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorMessage), nullptr);

		if (errorMessage && errorMessage->GetStringLength() > 0)
		{
			std::cout << errorMessage->GetStringPointer() << "\n";
		}

		auto* impl = new Shader::Impl();
		CheckHR(compileResult->GetResult(&impl->Handle));
		return { impl };
	}

}
