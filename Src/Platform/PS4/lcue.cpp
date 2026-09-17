
#include <gnmx/lcue.h>
#include <gnmx/lcue_validation.h>

using namespace sce;
using namespace Gnmx;

typedef struct
{
	uint8_t m_signature[7];					// 'OrbShdr'
	uint8_t m_version;						// ShaderBinaryInfoVersion
	unsigned int m_pssl_or_cg : 1;			// 1 = PSSL / Cg, 0 = IL / shtb
	unsigned int m_cached : 1;				// 1 = when compile, debugging source was cached.  May only make sense for PSSL=1
	uint32_t m_type : 4;					// See enum ShaderBinaryType
	uint32_t m_source_type : 2;				// See enum ShaderSourceType
	unsigned int m_length : 24;				// Binary length not counting this structure (i.e. points to top of binary)
	uint8_t  m_chunkUsageBaseOffsetInDW;	// in DW
	uint8_t  m_numInputUsageSlots;			// Up to 16 usage slots + 48 extended user data. (note: max of 63 since the ptr to the ext user data takes 2 slots)
	uint8_t  m_reserved3[2];				// For future use
	uint32_t m_shaderHash0;					// Association hash first 4 bytes
	uint32_t m_shaderHash1;					// Association hash second 4 bytes
	uint32_t m_crc32;						// crc32 of shader + this struct, just up till this field
} ShaderBinaryInfo;

// Profiling
#if defined(LCUE_PROFILE_ENABLED)
#define LCUE_PROFILE_SET_PTR_FETCH_CALL m_profiling.setPtrFetchShaderCount++
#define LCUE_PROFILE_SET_PTR_UED1_TABLE_CALL m_profiling.setPtrUserExtendedData1TableCount++
#define LCUE_PROFILE_SET_PTR_CBUFFER_TABLE_CALL m_profiling.setPtrConstantBufferTableCount++
#define LCUE_PROFILE_SET_PTR_VBUFFER_TABLE_CALL m_profiling.setPtrVertexBufferTableCount++
#define LCUE_PROFILE_SET_PTR_RESOURCE_TABLE_CALL m_profiling.setPtrResourceTableCount++
#define LCUE_PROFILE_SET_PTR_RWRESOURCE_TABLE_CALL m_profiling.setPtrRwResourceTableCount++
#define LCUE_PROFILE_SET_PTR_SAMPLER_TABLE_CALL m_profiling.setPtrSamplerTableCount++
#define LCUE_PROFILE_SET_PTR_USER_DATA_CALL m_profiling.setPtrUserDataCount++
#define LCUE_PROFILE_SET_PTR_GLOBAL_TABLE_CALL m_profiling.setPtrGlobalInternalTableCount++
#define LCUE_PROFILE_SET_PTR_APPEND_CONSUME_COUNTERS_CALL m_profiling.setPtrAppendConsumeCountersCount++
#define LCUE_PROFILE_SET_PTR_SOBUFFERS_CALL m_profiling.setStreamOutTableCount++
#define LCUE_PROFILE_SET_CBUFFERS_CALL m_profiling.setConstantBuffersCount++
#define LCUE_PROFILE_SET_VBUFFERS_CALL m_profiling.setVertexBuffersCount++
#define LCUE_PROFILE_SET_BUFFERS_CALL m_profiling.setBuffersCount++
#define LCUE_PROFILE_SET_RWBUFFERS_CALL m_profiling.setRwBuffersCount++
#define LCUE_PROFILE_SET_TEXTURES_CALL m_profiling.setTexturesCount++
#define LCUE_PROFILE_SET_RWTEXTURES_CALL m_profiling.setRwTexturesCount++
#define LCUE_PROFILE_SET_SAMPLERS_CALL m_profiling.setSamplersCount++
#define LCUE_PROFILE_SET_USER_DATA_CALL m_profiling.setUserDataCount++
#define LCUE_PROFILE_SET_SRT_USER_DATA_CALL m_profiling.setSrtUserDataCount++
#define LCUE_PROFILE_SET_INTERNAL_SRT_USER_DATA_CALL m_profiling.setInternalSrtUserDataCount++
#define LCUE_PROFILE_SET_APPEND_CONSUME_COUNTERS_CALL m_profiling.setAppendConsumeCountersCount++
#define LCUE_PROFILE_SET_SOBUFFERS_CALL m_profiling.setStreamOutsCount++
#define LCUE_PROFILE_SET_VSHARP_CALL m_profiling.setVsharpCount++
#define LCUE_PROFILE_SET_SSHARP_CALL m_profiling.setSsharpCount++
#define LCUE_PROFILE_SET_TSHARP_CALL m_profiling.setTsharpCount++
#define LCUE_PROFILE_DRAW_CALL m_profiling.drawCount++
#define LCUE_PROFILE_DISPATCH_CALL m_profiling.dispatchCount++
#define LCUE_PROFILE_REQUIRED_BUFFER_SIZE(TABLE) m_profiling.shaderResourceTotalUploadSizeInDwords += ((TABLE)->requiredBufferSizeInDwords)
#define LCUE_PROFILE_RESET __builtin_memset(&m_profiling, 0, sizeof(m_profiling))
#else
#define LCUE_PROFILE_SET_PTR_FETCH_CALL 
#define LCUE_PROFILE_SET_PTR_UED1_TABLE_CALL 
#define LCUE_PROFILE_SET_PTR_RESOURCE_TABLE_CALL 
#define LCUE_PROFILE_SET_PTR_RWRESOURCE_TABLE_CALL 
#define LCUE_PROFILE_SET_PTR_SAMPLER_TABLE_CALL 
#define LCUE_PROFILE_SET_PTR_USER_DATA_CALL
#define LCUE_PROFILE_SET_PTR_CBUFFER_TABLE_CALL 
#define LCUE_PROFILE_SET_PTR_VBUFFER_TABLE_CALL 
#define LCUE_PROFILE_SET_PTR_GLOBAL_TABLE_CALL 
#define LCUE_PROFILE_SET_PTR_APPEND_CONSUME_COUNTERS_CALL 
#define LCUE_PROFILE_SET_PTR_SOBUFFERS_CALL 
#define LCUE_PROFILE_SET_CBUFFERS_CALL 
#define LCUE_PROFILE_SET_VBUFFERS_CALL 
#define LCUE_PROFILE_SET_BUFFERS_CALL 
#define LCUE_PROFILE_SET_RWBUFFERS_CALL 
#define LCUE_PROFILE_SET_TEXTURES_CALL 
#define LCUE_PROFILE_SET_RWTEXTURES_CALL 
#define LCUE_PROFILE_SET_SAMPLERS_CALL 
#define LCUE_PROFILE_SET_USER_DATA_CALL
#define LCUE_PROFILE_SET_SRT_USER_DATA_CALL
#define LCUE_PROFILE_SET_INTERNAL_SRT_USER_DATA_CALL
#define LCUE_PROFILE_SET_APPEND_CONSUME_COUNTERS_CALL 
#define LCUE_PROFILE_SET_SOBUFFERS_CALL 
#define LCUE_PROFILE_SET_VSHARP_CALL 
#define LCUE_PROFILE_SET_SSHARP_CALL 
#define LCUE_PROFILE_SET_TSHARP_CALL 
#define LCUE_PROFILE_DRAW_CALL 
#define LCUE_PROFILE_DISPATCH_CALL 
#define LCUE_PROFILE_REQUIRED_BUFFER_SIZE(TABLE) 
#define LCUE_PROFILE_RESET
#endif

#if defined LCUE_IMMEDIATE_CB_AUTO_HANDLE_ENABLED
#define LCUE_IMMEDIATE_CB_UPDATE_ON_GRAPHICS(a, b) updateImmediateCb(a, (const Gnmx::ShaderCommonData*)(b))
#define LCUE_IMMEDIATE_CB_UPDATE_ON_COMPUTE(b) updateImmediateCb( (const Gnmx::ShaderCommonData*)(b) )
#else
#define LCUE_IMMEDIATE_CB_UPDATE_ON_GRAPHICS(a, b)
#define LCUE_IMMEDIATE_CB_UPDATE_ON_COMPUTE(b)
#endif

// Validation of resources that are not expected by the shader
#if defined LCUE_VALIDATE_RESOURCE_BINDING_AND_ASSERT_ENABLED
#define LCUE_ASSERT_OR_CONTINUE(a) LCUE_ASSERT(a)
#elif defined LCUE_VALIDATE_RESOURCE_BINDING_AND_SKIP_ENABLED
#define LCUE_ASSERT_OR_CONTINUE(a) if (!(a)) continue
#else
#define LCUE_ASSERT_OR_CONTINUE(a)
#endif

// Validation of complete resource binding
#if defined LCUE_VALIDATE_COMPLETE_RESOURCE_BINDING_ENABLED
#define LCUE_VALIDATE_RESOURCE_INIT_TABLE(a, b) initResourceBindingValidation(a, b)
#define LCUE_VALIDATE_RESOURCE_CHECK_TABLE(a) LCUE_ASSERT(isResourceBindingComplete(a))
#define LCUE_VALIDATE_RESOURCE_MARK_USED(a) (a) = true
#else
#define LCUE_VALIDATE_RESOURCE_INIT_TABLE(a, b)
#define LCUE_VALIDATE_RESOURCE_CHECK_TABLE(a)
#define LCUE_VALIDATE_RESOURCE_MARK_USED(a)
#endif


void setPersistentRegisterRange(Gnm::CommandBuffer* cb, Gnm::ShaderStage shaderStage, uint32_t startSgpr, const uint32_t* values, uint32_t valuesCount)
{
	LCUE_ASSERT( valuesCount <= (sizeof(Gnm::Texture)/sizeof(uint32_t)) );

	const uint32_t kGpuStageUserDataRegisterBases[Gnm::kShaderStageCount] = { 0x240, 0xC, 0x4C, 0x8C, 0xCC, 0x10C, 0x14C };
	Gnm::ShaderType shaderType = (shaderStage == Gnm::kShaderStageCs)? Gnm::kShaderTypeCompute : Gnm::kShaderTypeGraphics;

	uint32_t packetSizeInDw = 2 + valuesCount;

// Enable if you want to call the user-callback when the command buffer runs out of space.
#if 0
	bool hasSpace = cb->reserveSpaceInDwords(packetSizeInDw);
	LCUE_ASSERT( hasSpace );
#else
	LCUE_ASSERT( packetSizeInDw <= cb->getRemainingBufferSpaceInDwords());	
#endif

	uint32_t packet = 0xC0007600; // Set persistent register
	packet |= (shaderType << 1);
	packet |= (valuesCount << 16);

	cb->m_cmdptr[0] = packet;
	cb->m_cmdptr[1] = kGpuStageUserDataRegisterBases[(int32_t)shaderStage] + startSgpr;
	__builtin_memcpy(&cb->m_cmdptr[2], values, valuesCount * sizeof(uint32_t));
	cb->m_cmdptr += packetSizeInDw;
}


LCUE_FORCE_INLINE 
	void setPtrInPersistentRegister(Gnm::CommandBuffer* cb, Gnm::ShaderStage shaderStage, uint32_t startSgpr, const void* address)
{
	uint32_t values[2];
	values[0] = (uint32_t)( ((uintptr_t)address) & 0xFFFFFFFFULL );
	values[1] = (uint32_t)( ((uintptr_t)address) >> 32 );
	setPersistentRegisterRange(cb, shaderStage, startSgpr, values, 2);
}


LCUE_FORCE_INLINE 
	void setDataInUserDataSgprOrMemory(Gnm::CommandBuffer* dcb, uint32_t* scratchBuffer, Gnm::ShaderStage shaderStage, uint16_t shaderResourceOffset, const void* restrict data, int32_t dataSizeInBytes)
{
	LCUE_ASSERT(dcb != NULL);
	LCUE_ASSERT(data != NULL && dataSizeInBytes > 0 && (dataSizeInBytes%4) == 0);

	int32_t userDataRegisterOrMemoryOffset = (shaderResourceOffset & LCUE::kResourceValueMask);
	if ((shaderResourceOffset & LCUE::kResourceInUserDataSgpr) != 0)
	{
		setPersistentRegisterRange(dcb, shaderStage, userDataRegisterOrMemoryOffset, (uint32_t*)data, dataSizeInBytes/sizeof(uint32_t));
	}
	else
	{
		uint32_t* restrict scratchDestAddress = (uint32_t*)(scratchBuffer + ((int)shaderStage * LCUE::kGpuStageBufferSizeInDwords) + userDataRegisterOrMemoryOffset);
		__builtin_memcpy(scratchDestAddress, data, dataSizeInBytes);
	}
}


#if defined LCUE_VALIDATE_COMPLETE_RESOURCE_BINDING_ENABLED
bool isResourceBindingComplete(const LCUE::ShaderResourceBindingValidation* table)
{
	bool isValid = true;
	if (table != NULL)
	{
		int32_t i=0; // Failed resource index
		while (isValid && i<LCUE::kMaxConstantBufferCount) { isValid &= table->constBufferOffsetIsBound[i]; ++i; } --i;	LCUE_ASSERT(isValid); i=0;	// Vertex buffer not bound
		while (isValid && i<LCUE::kMaxVertexBufferCount) { isValid &= table->vertexBufferOffsetIsBound[i]; ++i; } --i;	LCUE_ASSERT(isValid); i=0;	// Constant buffer not bound
		while (isValid && i<LCUE::kMaxResourceCount) { isValid &= table->resourceOffsetIsBound[i]; ++i; } --i;			LCUE_ASSERT(isValid); i=0;	// Resource not bound
		while (isValid && i<LCUE::kMaxRwResourceCount) { isValid &= table->rwResourceOffsetIsBound[i]; ++i; } --i;		LCUE_ASSERT(isValid); i=0;	// RW-resource not bound
		while (isValid && i<LCUE::kMaxSamplerCount) { isValid &= table->samplerOffsetIsBound[i]; ++i; } --i;			LCUE_ASSERT(isValid); i=0;	// Sampler not bound
		//while (isValid && i<LCUE::kMaxStreamOutBufferCount) { isValid &= table->streamOutOffsetIsBound[i]; ++i; } --i;	LCUE_ASSERT(isValid); i=0;	// Stream-out not bound
		isValid &= table->appendConsumeCounterIsBound;																	LCUE_ASSERT(isValid);		// AppendConsumeCounter not bound

		// Note: if failing on Constant-Buffer slot 15, read comments for "LCUE_IMMEDIATE_CB_AUTO_HANDLE_ENABLED" (it might be related)
	}
	return isValid;
}


void initResourceBindingValidation(LCUE::ShaderResourceBindingValidation* validationTable, const LCUE::ShaderResourceOffsets* table)
{
	if (table != NULL)
	{
		// Mark all expected resources slots (!= 0xFFFF) as not bound (false)
		for (int32_t i=0; i<LCUE::kMaxConstantBufferCount; ++i)		validationTable->constBufferOffsetIsBound[i] = (table->constBufferDwOffset[i] == 0xFFFF);
		for (int32_t i=0; i<LCUE::kMaxVertexBufferCount; ++i)		validationTable->vertexBufferOffsetIsBound[i] = (table->vertexBufferDwOffset[i] == 0xFFFF);
		for (int32_t i=0; i<LCUE::kMaxResourceCount; ++i)			validationTable->resourceOffsetIsBound[i] = (table->resourceDwOffset[i] == 0xFFFF);
		for (int32_t i=0; i<LCUE::kMaxRwResourceCount; ++i)			validationTable->rwResourceOffsetIsBound[i] = (table->rwResourceDwOffset[i] == 0xFFFF);
		for (int32_t i=0; i<LCUE::kMaxSamplerCount; ++i)			validationTable->samplerOffsetIsBound[i] = (table->samplerDwOffset[i] == 0xFFFF);
		//for (int32_t i=0; i<LCUE::kMaxStreamOutBufferCount; ++i)	validationTable->streamOutOffsetIsBound[i] = (table->streamOutDwOffset[i] == 0xFFFF);
		validationTable->appendConsumeCounterIsBound = (table->appendConsumeCounterSgpr == 0xFF);
	}
	else
	{
		// If there's no table, all resources are valid
		LCUE_ASSERT( sizeof(bool) == sizeof(unsigned char) );
		__builtin_memset(validationTable, true, sizeof(LCUE::ShaderResourceBindingValidation));
	}
}
#endif


void LCUE::BaseCUE::init(
	uint32_t** resourceBuffersInGarlic, int32_t resourceBufferCount, int32_t resourceBufferSizeInDwords, void* globalInternalResourceTableInGarlic)
{
	LCUE_ASSERT(resourceBuffersInGarlic != NULL && resourceBufferCount >= 1 && resourceBufferCount <= kMaxResourceBufferCount && resourceBufferSizeInDwords >= kMinResourceBufferSizeInDwords);

	m_bufferIndex = 0;
	m_bufferCount = (resourceBufferCount < kMaxResourceBufferCount)? resourceBufferCount : kMaxResourceBufferCount;
	for (int32_t i=0; i<m_bufferCount; ++i)
	{
		m_bufferBegin[i] = resourceBuffersInGarlic[i];
		m_bufferEnd[i] = m_bufferBegin[i] + resourceBufferSizeInDwords;
	}
	m_bufferCurrent = m_bufferBegin[m_bufferIndex];

	m_globalInternalResourceTable = (Gnm::Buffer*)globalInternalResourceTableInGarlic;

	LCUE_PROFILE_RESET;
}


void LCUE::BaseCUE::swapBuffers()
{
	m_bufferIndex = (m_bufferIndex+1) % m_bufferCount;
	m_bufferCurrent = m_bufferBegin[m_bufferIndex];

	LCUE_PROFILE_RESET;
}


void LCUE::BaseCUE::setGlobalInternalResource(Gnm::ShaderGlobalResourceType resourceType, int32_t resourceCount, const Gnm::Buffer* restrict buffer)
{
	LCUE_ASSERT(m_globalInternalResourceTable != NULL && buffer != NULL);
	LCUE_ASSERT(resourceType >= 0 && resourceCount > 0 && resourceType+resourceCount <= Gnm::kShaderGlobalResourceCount);
	
	// Tessellation-factor-buffer - Checks address, alignment, size and memory type (apparently, must be Gnm::kResourceMemoryTypeGC)
	LCUE_ASSERT(resourceType != Gnm::kShaderGlobalResourceTessFactorBuffer || 
		(buffer->getBaseAddress() == sce::Gnm::getTheTessellationFactorRingBufferBaseAddress() &&
		((uintptr_t)buffer->getBaseAddress() % Gnm::kAlignmentOfTessFactorBufferInBytes) == 0 &&
		buffer->getSize() == Gnm::kTfRingSizeInBytes) );

	__builtin_memcpy(&m_globalInternalResourceTable[(int32_t)resourceType], buffer, resourceCount * sizeof(Gnm::Buffer));
}


void LCUE::ComputeCUE::init(uint32_t** resourceBuffersInGarlic, int32_t resourceBufferCount, int32_t resourceBufferSizeInDwords, void* globalInternalResourceTableInGarlic)
{
	BaseCUE::init(resourceBuffersInGarlic, resourceBufferCount, resourceBufferSizeInDwords, globalInternalResourceTableInGarlic);
	__builtin_memset(m_scratchBuffer, 0, sizeof(uint32_t) * kComputeScratchBufferSizeInDwords);

	m_dirtyShaderResources = false;
	m_dirtyShader = false;
	m_boundShaderResourceOffsets = NULL;
	m_boundShader = NULL;

	m_dcb = NULL;

#if defined LCUE_VALIDATE_COMPLETE_RESOURCE_BINDING_ENABLED
	__builtin_memset(&m_boundShaderResourcesValidation, true, sizeof(ShaderResourceBindingValidation));
#endif
}


void LCUE::ComputeCUE::swapBuffers()
{
	BaseCUE::swapBuffers();
	m_dirtyShaderResources = false;
	m_dirtyShader = false;
	m_boundShaderResourceOffsets = NULL;
	m_boundShader = NULL;

#if defined LCUE_VALIDATE_COMPLETE_RESOURCE_BINDING_ENABLED
	__builtin_memset(&m_boundShaderResourcesValidation, true, sizeof(ShaderResourceBindingValidation));
#endif
}


LCUE_FORCE_INLINE 
	uint32_t* LCUE::ComputeCUE::flushScratchBuffer()
{
	LCUE_ASSERT( m_boundShader != NULL );

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets;
	LCUE_ASSERT( (m_bufferCurrent + table->requiredBufferSizeInDwords) < m_bufferEnd[m_bufferIndex] );

	// Copy scratch data over the main buffer
	uint32_t* restrict destAddr = m_bufferCurrent;
	uint32_t* restrict sourceAddr = m_scratchBuffer;
	m_bufferCurrent += table->requiredBufferSizeInDwords;
	__builtin_memcpy(destAddr, sourceAddr, table->requiredBufferSizeInDwords * sizeof(uint32_t));

	LCUE_PROFILE_REQUIRED_BUFFER_SIZE(table);

	return destAddr;
}


LCUE_FORCE_INLINE 
	void LCUE::ComputeCUE::updateCommonPtrsInUserDataSgprs(const uint32_t* resourceBufferFlushedAddress)
{
	LCUE_ASSERT( m_boundShader != NULL );
	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets;

	if (table->userExtendedData1PtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, Gnm::kShaderStageCs, table->userExtendedData1PtrSgpr, resourceBufferFlushedAddress);
		LCUE_PROFILE_SET_PTR_UED1_TABLE_CALL;
	}
	if (table->constBufferPtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, Gnm::kShaderStageCs, table->constBufferPtrSgpr, resourceBufferFlushedAddress + table->constBufferArrayDwOffset);
		LCUE_PROFILE_SET_PTR_CBUFFER_TABLE_CALL;
	}
	if (table->resourcePtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, Gnm::kShaderStageCs, table->resourcePtrSgpr, resourceBufferFlushedAddress + table->resourceArrayDwOffset);
		LCUE_PROFILE_SET_PTR_RESOURCE_TABLE_CALL;
	}
	if (table->rwResourcePtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, Gnm::kShaderStageCs, table->rwResourcePtrSgpr, resourceBufferFlushedAddress + table->rwResourceArrayDwOffset);
		LCUE_PROFILE_SET_PTR_RWRESOURCE_TABLE_CALL;
	}
	if (table->samplerPtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, Gnm::kShaderStageCs, table->samplerPtrSgpr, resourceBufferFlushedAddress + table->samplerArrayDwOffset);
		LCUE_PROFILE_SET_PTR_SAMPLER_TABLE_CALL;
	}
	if (table->globalInternalPtrSgpr != 0xFF)
	{
		LCUE_ASSERT(m_globalInternalResourceTable != NULL);
		setPtrInPersistentRegister(m_dcb, Gnm::kShaderStageCs, table->globalInternalPtrSgpr, m_globalInternalResourceTable);

		// TODOBE check if required entries are set and are valid

		LCUE_PROFILE_SET_PTR_GLOBAL_TABLE_CALL;
	}
	if (table->appendConsumeCounterSgpr != 0xFF)
	{
		setPersistentRegisterRange(m_dcb, Gnm::kShaderStageCs, table->appendConsumeCounterSgpr, &m_boundShaderAppendConsumeCounterRange, 1);
		LCUE_PROFILE_SET_PTR_APPEND_CONSUME_COUNTERS_CALL;
	}
}


LCUE_FORCE_INLINE 
	void LCUE::ComputeCUE::updateImmediateCb(const Gnmx::ShaderCommonData* shaderCommon)
{
	if (shaderCommon != NULL && shaderCommon->m_embeddedConstantBufferSizeInDQW > 0)
	{
		const uint32_t* shaderRegisters = (const uint32_t*)(shaderCommon + 1);
		const uint8_t* shaderCode = (uint8_t*)( ((uintptr_t)shaderRegisters[0] << 8ULL) | ((uintptr_t)shaderRegisters[1] << 40ULL) );

		Gnm::Buffer embeddedCb;
		embeddedCb.initAsConstantBuffer((void*)(shaderCode + shaderCommon->m_shaderSize), shaderCommon->m_embeddedConstantBufferSizeInDQW << 4);
		setConstantBuffers(LCUE::kConstantBufferInternalApiSlotForEmbeddedData, 1, &embeddedCb);
	}
}


void LCUE::ComputeCUE::preDispatch()
{
	LCUE_ASSERT(m_dcb != NULL);

	const Gnmx::CsShader* csShader = (const Gnmx::CsShader*)m_boundShader;
	if (m_dirtyShader)
		m_dcb->setCsShader( &csShader->m_csStageRegisters );

	// Handle Immediate Constant Buffer on CB slot 15
	if (m_dirtyShader || m_dirtyShaderResources)
		LCUE_IMMEDIATE_CB_UPDATE_ON_COMPUTE(csShader);

	if (m_dirtyShaderResources)
	{
		LCUE_VALIDATE_RESOURCE_CHECK_TABLE( &m_boundShaderResourcesValidation );
		updateCommonPtrsInUserDataSgprs( flushScratchBuffer() );
	}

	m_dirtyShaderResources = false;
	m_dirtyShader = false;
	LCUE_PROFILE_DISPATCH_CALL;
}


void LCUE::ComputeCUE::setCsShader(const Gnmx::CsShader* shader, const ShaderResourceOffsets* table)
{
	LCUE_ASSERT( shader != NULL && table != NULL && table->shaderStage == Gnm::kShaderStageCs );
	LCUE_ASSERT( (m_bufferCurrent + table->requiredBufferSizeInDwords) < m_bufferEnd[m_bufferIndex] );
	LCUE_VALIDATE_RESOURCE_INIT_TABLE( &m_boundShaderResourcesValidation, table );

	m_dirtyShaderResources = true;
	m_dirtyShader |= (m_boundShader != shader);
	m_boundShaderResourceOffsets = table;
	m_boundShader = shader;
}


void LCUE::ComputeCUE::setAppendConsumeCounterRange(uint32_t gdsMemoryBaseInBytes, uint32_t countersSizeInBytes)
{
	LCUE_ASSERT((gdsMemoryBaseInBytes%4)==0 && gdsMemoryBaseInBytes < UINT16_MAX && (countersSizeInBytes%4)==0 && countersSizeInBytes < UINT16_MAX);
	LCUE_ASSERT(m_boundShaderResourceOffsets != NULL);

	do
	{
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets->appendConsumeCounterSgpr != 0xFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation.appendConsumeCounterIsBound);
		m_boundShaderAppendConsumeCounterRange = (gdsMemoryBaseInBytes << 16) | countersSizeInBytes;
	} while (false);

	LCUE_PROFILE_SET_APPEND_CONSUME_COUNTERS_CALL;
	m_dirtyShaderResources = true;
}


void LCUE::ComputeCUE::setConstantBuffers(int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Buffer* restrict buffer)
{
	LCUE_ASSERT(startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxConstantBufferCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets != NULL && buffer != NULL && buffer->isBuffer());

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets;
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_CONSTANT_BUFFER(buffer+i);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets->constBufferDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED( m_boundShaderResourcesValidation.constBufferOffsetIsBound[currentApiSlot] );
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, Gnm::kShaderStageCs, table->constBufferDwOffset[currentApiSlot], buffer+i, sizeof(Gnm::Buffer));
		LCUE_PROFILE_SET_VSHARP_CALL;
	}

	LCUE_PROFILE_SET_CBUFFERS_CALL;
	m_dirtyShaderResources = true;
}


void LCUE::ComputeCUE::setBuffers(int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Buffer* restrict buffer)
{
	LCUE_ASSERT(startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxResourceCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets != NULL && buffer != NULL && buffer->isBuffer());

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets;
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_BUFFER(buffer+i);
		// Cannot currently be validated as this information is not stored for entries in the resource-flat-table (possible to retrieve from Shader::Binary)
		//LCUE_ASSERT((m_boundShaderResourceOffsets[(int32_t)shaderStage]->resourceDwOffset[currentApiSlot] & kResourceIsVSharp) != 0);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets->resourceDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation.resourceOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, Gnm::kShaderStageCs, table->resourceDwOffset[currentApiSlot], buffer+i, sizeof(Gnm::Buffer));
		LCUE_PROFILE_SET_VSHARP_CALL;
	}

	LCUE_PROFILE_SET_BUFFERS_CALL;
	m_dirtyShaderResources = true;
}


void LCUE::ComputeCUE::setRwBuffers(int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Buffer* restrict buffer)
{
	LCUE_ASSERT(startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxRwResourceCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets != NULL && buffer != NULL && buffer->isBuffer());
	LCUE_ASSERT(buffer->getResourceMemoryType() != Gnm::kResourceMemoryTypeRO);
	
	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets;
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_RWBUFFER(buffer+i);
		// Cannot currently be validated as this information is not stored for entries in the resource-flat-table (possible to retrieve from Shader::Binary)
		//LCUE_ASSERT((m_boundShaderResourceOffsets[(int32_t)shaderStage]->rwResourceDwOffset[currentApiSlot] & kResourceIsVSharp) != 0);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets->rwResourceDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation.rwResourceOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, Gnm::kShaderStageCs, table->rwResourceDwOffset[currentApiSlot], buffer+i, sizeof(Gnm::Buffer));
		LCUE_PROFILE_SET_VSHARP_CALL;
	}

	LCUE_PROFILE_SET_RWBUFFERS_CALL;
	m_dirtyShaderResources = true;
}


void LCUE::ComputeCUE::setTextures(int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Texture* restrict texture)
{
	LCUE_ASSERT(startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxResourceCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets != NULL && texture != NULL && texture->isTexture());

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets;
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_TEXTURE(texture+i);
		// Cannot currently be validated as this information is not stored for entries in the resource-flat-table (possible to retrieve from Shader::Binary)
		//LCUE_ASSERT((m_boundShaderResourceOffsets[(int32_t)shaderStage]->resourceDwOffset[currentApiSlot] & kResourceIsVSharp) == 0);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets->resourceDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation.resourceOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, Gnm::kShaderStageCs, table->resourceDwOffset[currentApiSlot], texture+i, sizeof(Gnm::Texture));
		LCUE_PROFILE_SET_TSHARP_CALL;
	}

	LCUE_PROFILE_SET_TEXTURES_CALL;
	m_dirtyShaderResources = true;
}


void LCUE::ComputeCUE::setRwTextures(int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Texture* restrict texture)
{
	LCUE_ASSERT(startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxRwResourceCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets != NULL && texture != NULL && texture->isTexture());
	LCUE_ASSERT(texture->getResourceMemoryType() != Gnm::kResourceMemoryTypeRO);

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets;
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_RWTEXTURE(texture+i);
		// Cannot currently be validated as this information is not stored for entries in the resource-flat-table (possible to retrieve from Shader::Binary)
		//LCUE_ASSERT((m_boundShaderResourceOffsets[(int32_t)shaderStage]->rwResourceDwOffset[currentApiSlot] & kResourceIsVSharp) == 0);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets->rwResourceDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation.rwResourceOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, Gnm::kShaderStageCs, table->rwResourceDwOffset[currentApiSlot], texture+i, sizeof(Gnm::Texture));
		LCUE_PROFILE_SET_TSHARP_CALL;
	}

	LCUE_PROFILE_SET_RWTEXTURES_CALL;
	m_dirtyShaderResources = true;
}


void LCUE::ComputeCUE::setSamplers(int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Sampler* restrict sampler)
{
	LCUE_ASSERT(startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxSamplerCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets != NULL && sampler != NULL);

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets;
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_SAMPLER(sampler+i);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets->samplerDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation.samplerOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, Gnm::kShaderStageCs, table->samplerDwOffset[currentApiSlot], sampler+i, sizeof(Gnm::Sampler));
		LCUE_PROFILE_SET_SSHARP_CALL;
	}

	LCUE_PROFILE_SET_SAMPLERS_CALL;
	m_dirtyShaderResources = true;
}


void LCUE::ComputeCUE::setUserData(int32_t startSgpr, int32_t sgprCount, const uint32_t* data)
{
	LCUE_ASSERT(startSgpr >= 0 && (startSgpr+sgprCount) <= kMaxUserDataCount);
	setPersistentRegisterRange(m_dcb, Gnm::kShaderStageCs, startSgpr, data, sgprCount);
	LCUE_PROFILE_SET_USER_DATA_CALL;
}


void LCUE::ComputeCUE::setPtrInUserData(int32_t startSgpr, const void* gpuAddress)
{
	LCUE_ASSERT(startSgpr >= 0 && (startSgpr) <= kMaxUserDataCount);
	setPtrInPersistentRegister(m_dcb, Gnm::kShaderStageCs, startSgpr, gpuAddress);
	LCUE_PROFILE_SET_PTR_USER_DATA_CALL;
}


void LCUE::ComputeCUE::setUserSrtBuffer(const void* buffer, uint32_t sizeInDwords)
{
	LCUE_ASSERT(sizeInDwords > 0 && sizeInDwords <= kMaxSrtUserDataCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets != NULL && buffer != NULL);

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets;
	LCUE_ASSERT(sizeInDwords == table->userSrtDataCount);
	setUserData(table->userSrtDataSgpr, table->userSrtDataCount, (const uint32_t *)buffer);
	LCUE_PROFILE_SET_SRT_USER_DATA_CALL;
}


void LCUE::ComputeCUE::setInternalSrtBuffer(const void* buffer)
{
	LCUE_ASSERT(false); // Not supported, the LCUE already binds directly to the EUD via the scratch buffer 

// 	LCUE_ASSERT(m_boundShaderResourceOffsets != NULL && buffer != NULL);
// 
// 	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets;
// 	setPtrInUserData(table->userInternalSrtDataPtrSgpr, buffer);
// 	LCUE_PROFILE_SET_INTERNAL_SRT_USER_DATA_CALL;
}


void LCUE::GraphicsCUE::init(uint32_t** resourceBuffersInGarlic, int32_t resourceBufferCount, int32_t resourceBufferSizeInDwords, void* globalInternalResourceTableInGarlic)
{
	BaseCUE::init(resourceBuffersInGarlic, resourceBufferCount, resourceBufferSizeInDwords, globalInternalResourceTableInGarlic);
	__builtin_memset(m_scratchBuffer, 0, sizeof(uint32_t) * kGraphicsScratchBufferSizeInDwords);

	__builtin_memset(m_dirtyShaderResources, 0, sizeof(bool) * Gnm::kShaderStageCount);
	__builtin_memset(m_dirtyShader, 0, sizeof(bool) * Gnm::kShaderStageCount);
	__builtin_memset(m_boundShaderResourceOffsets, 0, sizeof(void*) * Gnm::kShaderStageCount);
	__builtin_memset(m_boundShader, 0, sizeof(void*) * Gnm::kShaderStageCount);
	m_dirtyShader[Gnm::kShaderStagePs] = true; // First Pixel Shader MUST be marked as dirty (handles issue when first PS is NULL)

	m_dcb = NULL;
	m_activeShaderStages = Gnm::kActiveShaderStagesVsPs;

#if defined LCUE_GEOMETRY_SHADERS_ENABLED
	// Fixed offset table for the VS GPU stage when the Geometry pipeline is enabled (the copy shader)
	__builtin_memset(&m_fixedGsVsShaderResourceOffsets, 0xFF, sizeof(ShaderResourceOffsets));
	__builtin_memset(&m_fixedGsVsStreamOutShaderResourceOffsets, 0xFF, sizeof(ShaderResourceOffsets));
	m_fixedGsVsShaderResourceOffsets.requiredBufferSizeInDwords = 0;
	m_fixedGsVsShaderResourceOffsets.shaderStage = Gnm::kShaderStageVs;
	m_fixedGsVsShaderResourceOffsets.globalInternalPtrSgpr = kDefaultGlobalInternalTablePtrSgpr;
	m_fixedGsVsStreamOutShaderResourceOffsets.requiredBufferSizeInDwords = kMaxStreamOutBufferCount * sizeof(Gnm::Buffer); // 4 stream-out buffers
	m_fixedGsVsStreamOutShaderResourceOffsets.shaderStage = Gnm::kShaderStageVs;
	m_fixedGsVsStreamOutShaderResourceOffsets.globalInternalPtrSgpr = kDefaultGlobalInternalTablePtrSgpr;
	m_fixedGsVsStreamOutShaderResourceOffsets.streamOutPtrSgpr = kDefaultStreamOutTablePtrSgpr;
	m_fixedGsVsStreamOutShaderResourceOffsets.streamOutArrayDwOffset = 0;
	for (int32_t i=0; i<kMaxStreamOutBufferCount; i++)
		m_fixedGsVsStreamOutShaderResourceOffsets.streamOutDwOffset[i] = i * sizeof(Gnm::Buffer) / sizeof(uint32_t);
#endif

	m_gsMode = Gnm::kGsModeDisable;
	m_gsMaxOutput = Gnm::kGsMaxOutputVertexCount1024;

	m_tessellationDesiredTgPatchCount = 0;

#if defined LCUE_VALIDATE_COMPLETE_RESOURCE_BINDING_ENABLED
	__builtin_memset(m_boundShaderResourcesValidation, true, sizeof(ShaderResourceBindingValidation) * Gnm::kShaderStageCount);
#endif
}


void LCUE::GraphicsCUE::swapBuffers()
{
	BaseCUE::swapBuffers();
	__builtin_memset(m_dirtyShaderResources, 0, sizeof(bool) * Gnm::kShaderStageCount);
	__builtin_memset(m_dirtyShader, 0, sizeof(bool) * Gnm::kShaderStageCount);
	__builtin_memset(m_boundShaderResourceOffsets, 0, sizeof(void*) * Gnm::kShaderStageCount);
	__builtin_memset(m_boundShader, 0, sizeof(void*) * Gnm::kShaderStageCount);
	m_dirtyShader[Gnm::kShaderStagePs] = true; // First Pixel Shader MUST be marked as dirty (handles issue when first PS is NULL)

#if defined LCUE_VALIDATE_COMPLETE_RESOURCE_BINDING_ENABLED
	__builtin_memset(m_boundShaderResourcesValidation, true, sizeof(ShaderResourceBindingValidation) * Gnm::kShaderStageCount);
#endif
}


LCUE_FORCE_INLINE 
	uint32_t* LCUE::GraphicsCUE::flushScratchBuffer(Gnm::ShaderStage shaderStage)
{
	LCUE_ASSERT( m_boundShader[(int32_t)shaderStage] != NULL );

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];
	LCUE_ASSERT( (m_bufferCurrent + table->requiredBufferSizeInDwords) < m_bufferEnd[m_bufferIndex] );

	// Copy scratch data over the main buffer
	uint32_t* restrict destAddr = m_bufferCurrent;
	uint32_t* restrict sourceAddr = m_scratchBuffer + ((int32_t)shaderStage * kGpuStageBufferSizeInDwords);
	m_bufferCurrent += table->requiredBufferSizeInDwords;
	__builtin_memcpy(destAddr, sourceAddr, table->requiredBufferSizeInDwords * sizeof(uint32_t));

	LCUE_PROFILE_REQUIRED_BUFFER_SIZE(table);

	return destAddr;
}


LCUE_FORCE_INLINE 
	void LCUE::GraphicsCUE::updateLsEsVsPtrsInUserDataSgprs(Gnm::ShaderStage shaderStage, const uint32_t* resourceBufferFlushedAddress)
{
	LCUE_ASSERT( m_boundShader[(int32_t)shaderStage] != NULL );
	LCUE_ASSERT( shaderStage == Gnm::kShaderStageLs || shaderStage == Gnm::kShaderStageEs || shaderStage == Gnm::kShaderStageVs );
	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];

	if (table->fetchShaderPtrSgpr != 0xFF)
	{
		LCUE_ASSERT( m_boundFetchShader[(int32_t)shaderStage] != NULL );
		setPtrInPersistentRegister(m_dcb, shaderStage, table->fetchShaderPtrSgpr, m_boundFetchShader[(int32_t)shaderStage]);
		LCUE_PROFILE_SET_PTR_FETCH_CALL;
	}
	if (table->vertexBufferPtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, shaderStage, table->vertexBufferPtrSgpr, resourceBufferFlushedAddress + table->vertexBufferArrayDwOffset );
		LCUE_PROFILE_SET_PTR_VBUFFER_TABLE_CALL;
	}
#if defined LCUE_GEOMETRY_SHADERS_ENABLED
	if (table->streamOutPtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, shaderStage, table->streamOutPtrSgpr, resourceBufferFlushedAddress + table->streamOutArrayDwOffset);
		LCUE_PROFILE_SET_PTR_SOBUFFERS_CALL;
	}
#endif
}


LCUE_FORCE_INLINE 
	void LCUE::GraphicsCUE::updateCommonPtrsInUserDataSgprs(Gnm::ShaderStage shaderStage, const uint32_t* resourceBufferFlushedAddress)
{
	LCUE_ASSERT( m_boundShader[(int32_t)shaderStage] != NULL );
	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];
	
	if (table->userExtendedData1PtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, shaderStage, table->userExtendedData1PtrSgpr, resourceBufferFlushedAddress );
		LCUE_PROFILE_SET_PTR_UED1_TABLE_CALL;
	}
	if (table->constBufferPtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, shaderStage, table->constBufferPtrSgpr, resourceBufferFlushedAddress + table->constBufferArrayDwOffset );
		LCUE_PROFILE_SET_PTR_CBUFFER_TABLE_CALL;
	}
	if (table->resourcePtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, shaderStage, table->resourcePtrSgpr, resourceBufferFlushedAddress + table->resourceArrayDwOffset );
		LCUE_PROFILE_SET_PTR_RESOURCE_TABLE_CALL;
	}
	if (table->rwResourcePtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, shaderStage, table->rwResourcePtrSgpr, resourceBufferFlushedAddress + table->rwResourceArrayDwOffset );
		LCUE_PROFILE_SET_PTR_RWRESOURCE_TABLE_CALL;
	}
	if (table->samplerPtrSgpr != 0xFF)
	{
		setPtrInPersistentRegister(m_dcb, shaderStage, table->samplerPtrSgpr, resourceBufferFlushedAddress + table->samplerArrayDwOffset );
		LCUE_PROFILE_SET_PTR_SAMPLER_TABLE_CALL;
	}
	if (table->globalInternalPtrSgpr != 0xFF)
	{
		LCUE_ASSERT(m_globalInternalResourceTable != NULL);
		setPtrInPersistentRegister(m_dcb, shaderStage, table->globalInternalPtrSgpr, m_globalInternalResourceTable );
		LCUE_PROFILE_SET_PTR_GLOBAL_TABLE_CALL;
	}
	if (table->appendConsumeCounterSgpr != 0xFF)
	{
		setPersistentRegisterRange(m_dcb, shaderStage, table->appendConsumeCounterSgpr, &m_boundShaderAppendConsumeCounterRange[shaderStage], 1);
		LCUE_PROFILE_SET_PTR_APPEND_CONSUME_COUNTERS_CALL;
	}

}


LCUE_FORCE_INLINE 
	void LCUE::GraphicsCUE::updateImmediateCb(sce::Gnm::ShaderStage shaderStage, const Gnmx::ShaderCommonData* shaderCommon)
{
	if (shaderCommon != NULL && shaderCommon->m_embeddedConstantBufferSizeInDQW > 0)
	{
		const uint32_t* shaderRegisters = (const uint32_t*)(shaderCommon + 1);
		const uint8_t* shaderCode = (uint8_t*)( ((uintptr_t)shaderRegisters[0] << 8ULL) | ((uintptr_t)shaderRegisters[1] << 40ULL) );

		Gnm::Buffer embeddedCb;
		embeddedCb.initAsConstantBuffer((void*)(shaderCode + shaderCommon->m_shaderSize), shaderCommon->m_embeddedConstantBufferSizeInDQW << 4);
		setConstantBuffers(shaderStage, LCUE::kConstantBufferInternalApiSlotForEmbeddedData, 1, &embeddedCb);
	}
}


void LCUE::GraphicsCUE::preDispatch()
{
	LCUE_ASSERT(m_dcb != NULL);
	
	const Gnmx::CsShader* csShader = ((const Gnmx::CsShader*)m_boundShader[Gnm::kShaderStageCs]);
	if (m_dirtyShader[Gnm::kShaderStageCs])
		m_dcb->setCsShader( &csShader->m_csStageRegisters );

	// Handle Immediate Constant Buffer on CB slot 15
	if (m_dirtyShader[Gnm::kShaderStageCs] || m_dirtyShaderResources[Gnm::kShaderStageCs])
		LCUE_IMMEDIATE_CB_UPDATE_ON_GRAPHICS(Gnm::kShaderStageCs, csShader);

	if (m_dirtyShaderResources[Gnm::kShaderStageCs])
	{
		LCUE_VALIDATE_RESOURCE_CHECK_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStageCs] );

		const uint32_t* flushAddress = flushScratchBuffer(Gnm::kShaderStageCs);
		updateCommonPtrsInUserDataSgprs(Gnm::kShaderStageCs, flushAddress);
	}

	m_dirtyShaderResources[Gnm::kShaderStageCs] = false;
	m_dirtyShader[Gnm::kShaderStageCs] = false;
	LCUE_PROFILE_DISPATCH_CALL;
}


LCUE_FORCE_INLINE 
	void LCUE::GraphicsCUE::preDrawTessellation(bool geometryShaderEnabled)
{
	(void)geometryShaderEnabled;

	#if defined LCUE_TESSELLATION_PSSLC_BUG_WORKAROUND_ENABLED
		if (m_dirtyShader[Gnm::kShaderStageHs])
		{
			LCUE_ASSERT(m_boundShader[Gnm::kShaderStageHs] != NULL);
	
			// Note that we are changing a const value here but this is workaround for a PSSL compiler bug
			Gnmx::HsShader* hsShader = (Gnmx::HsShader*)m_boundShader[Gnm::kShaderStageHs];
			if (hsShader->m_hsStageRegisters.m_vgtHosMaxTessLevel == 0)
				hsShader->m_hsStageRegisters.m_vgtHosMaxTessLevel = (6 + 127) << 23; // 64.0f
		}
	#endif

	const Gnmx::LsShader* lsShader = (const Gnmx::LsShader*)m_boundShader[Gnm::kShaderStageLs];
	const Gnmx::HsShader* hsShader = (const Gnmx::HsShader*)m_boundShader[Gnm::kShaderStageHs];

	int32_t tgPatchCount = m_tessellationDesiredTgPatchCount;
	const Gnm::LsStageRegisters* lsStateRegisters = &lsShader->m_lsStageRegisters;

#if defined LCUE_TESSELLATION_TG_PATCH_COUNT_AUTO_CONFIGURE_ENABLED
	Gnm::LsStageRegisters lsStateRegistersCopy;
	if (m_dirtyShader[Gnm::kShaderStageLs] || m_dirtyShader[Gnm::kShaderStageHs])
	{
		LCUE_ASSERT(m_boundShader[Gnm::kShaderStageLs] != NULL);
		LCUE_ASSERT(m_boundShader[Gnm::kShaderStageHs] != NULL);

		int32_t tgLdsSizeInBytes;
		computeTessellationTgPatchCountAndLdsSize(&tgPatchCount, &tgLdsSizeInBytes, lsShader, hsShader, m_tessellationDesiredTgPatchCount);

		// Update required LDS size on LS resource register
		lsStateRegistersCopy = lsShader->m_lsStageRegisters;
		lsStateRegistersCopy.m_spiShaderPgmRsrc2Ls |= (tgLdsSizeInBytes >> 2); // TODO Needs to zero bitfield first?
		lsStateRegisters = &lsStateRegistersCopy;

		// Generate internal TessellationDataConstantBuffer
		LCUE_ASSERT( sizeof(Gnm::TessellationDataConstantBuffer) % 4 == 0);
		LCUE_ASSERT( (m_bufferCurrent + sizeof(Gnm::TessellationDataConstantBuffer) / 4) < m_bufferEnd[m_bufferIndex] );

		Gnm::TessellationDataConstantBuffer* tessellationInternalConstantBuffer = (Gnm::TessellationDataConstantBuffer*)m_bufferCurrent;
		m_bufferCurrent += sizeof(Gnm::TessellationDataConstantBuffer) / 4;
		tessellationInternalConstantBuffer->init(&hsShader->m_hullStateConstants, lsShader->m_lsStride, tgPatchCount); // TODO: I can manually inline this to speedup things
		
		// Workaround for passthrough HS while Gnm doesn't correctly handle it
		// TODOBE Remove on SDK 1.2.0
		if (hsShader->m_hullStateConstants.m_cpStride == 0)
			tessellationInternalConstantBuffer->m_patchOutputSize = hsShader->m_hullStateConstants.m_numOutputCP * lsShader->m_lsStride;

		// Update tessellation internal CB on API-slot 19
		m_tessellationCurrentCb.initAsConstantBuffer(tessellationInternalConstantBuffer, sizeof(Gnm::TessellationDataConstantBuffer));

		// Update VGT_LS_HS context registers
		m_dcb->setVgtControl(tgPatchCount-1, Gnm::kVgtPartialVsWaveEnable, Gnm::kVgtSwitchOnEopEnable);
	}

	setConstantBuffers(Gnm::kShaderStageHs, kConstantBufferInternalApiSlotForTessellation, 1, &m_tessellationCurrentCb);
	setConstantBuffers( (geometryShaderEnabled)? Gnm::kShaderStageEs : Gnm::kShaderStageVs, kConstantBufferInternalApiSlotForTessellation, 1, &m_tessellationCurrentCb);

#endif

	if (m_dirtyShader[Gnm::kShaderStageLs])
		m_dcb->setLsShader( lsStateRegisters, m_boundShaderModifier[Gnm::kShaderStageLs] );

	if (m_dirtyShader[Gnm::kShaderStageHs])
	{
		Gnm::TessellationRegisters tessellationVgtLsHsConfiguration;
		tessellationVgtLsHsConfiguration.init( &hsShader->m_hullStateConstants, tgPatchCount);
		m_dcb->setHsShader( &hsShader->m_hsStageRegisters, &tessellationVgtLsHsConfiguration );
	}

	// Handle Immediate Constant Buffer on CB slot 15
	if (m_dirtyShader[Gnm::kShaderStageLs] || m_dirtyShaderResources[Gnm::kShaderStageLs])
		LCUE_IMMEDIATE_CB_UPDATE_ON_GRAPHICS(Gnm::kShaderStageLs, lsShader);
	if (m_dirtyShader[Gnm::kShaderStageHs] || m_dirtyShaderResources[Gnm::kShaderStageHs])
		LCUE_IMMEDIATE_CB_UPDATE_ON_GRAPHICS(Gnm::kShaderStageHs, hsShader);

	if (m_dirtyShaderResources[Gnm::kShaderStageLs])
	{
		// Validate after handling immediate CBs (if necessary) and before flushing scratch buffer
		LCUE_VALIDATE_RESOURCE_CHECK_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStageLs] );

		const uint32_t* flushAddress = flushScratchBuffer(Gnm::kShaderStageLs);
		updateCommonPtrsInUserDataSgprs(Gnm::kShaderStageLs, flushAddress);
		updateLsEsVsPtrsInUserDataSgprs(Gnm::kShaderStageLs, flushAddress);
	}
	if (m_dirtyShaderResources[Gnm::kShaderStageHs])
	{
		// Validate after handling immediate CBs (if necessary) and before flushing scratch buffer
		LCUE_VALIDATE_RESOURCE_CHECK_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStageHs] );

		const uint32_t* flushAddress = flushScratchBuffer(Gnm::kShaderStageHs);
		updateCommonPtrsInUserDataSgprs(Gnm::kShaderStageHs, flushAddress);
	}
}


LCUE_FORCE_INLINE
	void LCUE::GraphicsCUE::preDrawGeometryShader()
{
	const Gnmx::EsShader* esShader = (const Gnmx::EsShader*)m_boundShader[Gnm::kShaderStageEs];
	const Gnmx::GsShader* gsShader = (const Gnmx::GsShader*)m_boundShader[Gnm::kShaderStageGs];

	if (m_dirtyShader[Gnm::kShaderStageEs])
		m_dcb->setEsShader( &esShader->m_esStageRegisters, m_boundShaderModifier[Gnm::kShaderStageEs] );
	if (m_dirtyShader[Gnm::kShaderStageGs])
		m_dcb->setGsShader( &gsShader->m_gsStageRegisters );

	// Handle Immediate Constant Buffer on CB slot 15
	if (m_dirtyShader[Gnm::kShaderStageEs] || m_dirtyShaderResources[Gnm::kShaderStageEs])
		LCUE_IMMEDIATE_CB_UPDATE_ON_GRAPHICS(Gnm::kShaderStageEs, esShader);
	if (m_dirtyShader[Gnm::kShaderStageGs] || m_dirtyShaderResources[Gnm::kShaderStageGs])
		LCUE_IMMEDIATE_CB_UPDATE_ON_GRAPHICS(Gnm::kShaderStageGs, gsShader);

	if (m_dirtyShaderResources[Gnm::kShaderStageEs])
	{
		// Validate after handling immediate CBs (if necessary) and before flushing scratch buffer
		LCUE_VALIDATE_RESOURCE_CHECK_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStageEs] );

		const uint32_t* flushAddress = flushScratchBuffer(Gnm::kShaderStageEs);
		updateCommonPtrsInUserDataSgprs(Gnm::kShaderStageEs, flushAddress);
		updateLsEsVsPtrsInUserDataSgprs(Gnm::kShaderStageEs, flushAddress);
	}
	if (m_dirtyShaderResources[Gnm::kShaderStageGs])
	{
		// Validate after handling immediate CBs (if necessary) and before flushing scratch buffer
		LCUE_VALIDATE_RESOURCE_CHECK_TABLE(	&m_boundShaderResourcesValidation[Gnm::kShaderStageGs] );

		const uint32_t* flushAddress = flushScratchBuffer(Gnm::kShaderStageGs);
		updateCommonPtrsInUserDataSgprs(Gnm::kShaderStageGs, flushAddress);
	}

	// This rolls the hardware context, thus we should only set it if it's really necessary
	LCUE_ASSERT((Gnm::GsMaxOutputVertexCount)gsShader->getCopyShader()->m_gsModeOrNumInputSemanticsCs <= Gnm::kGsMaxOutputVertexCount128);
	Gnm::GsMaxOutputVertexCount requiredGsMaxOutput = (Gnm::GsMaxOutputVertexCount)gsShader->getCopyShader()->m_gsModeOrNumInputSemanticsCs;
	if (m_gsMode != Gnm::kGsModeEnable || requiredGsMaxOutput != m_gsMaxOutput)
	{
		m_dcb->setGsMode(Gnm::kGsModeEnable, requiredGsMaxOutput);
	}
	m_gsMode = Gnm::kGsModeEnable;
	m_gsMaxOutput = requiredGsMaxOutput;
}


void LCUE::GraphicsCUE::preDraw()
{
	LCUE_ASSERT( m_dcb != NULL );
	LCUE_ASSERT( m_boundShader[Gnm::kShaderStageVs] != NULL); // VS cannot be NULL for any pipeline configuration
	//LCUE_ASSERT( m_boundShader[Gnm::kShaderStagePs] != NULL); // Pixel Shader can be NULL

	bool tessellationEnabled = false;
	bool geometryShaderEnabled = false;
	(void)tessellationEnabled;
	(void)geometryShaderEnabled;

#if defined(LCUE_GEOMETRY_SHADERS_ENABLED)
	geometryShaderEnabled = (m_activeShaderStages == Gnm::kActiveShaderStagesEsGsVsPs) | (m_activeShaderStages == Gnm::kActiveShaderStagesLsHsEsGsVsPs);
	if (geometryShaderEnabled)
		preDrawGeometryShader();
#endif

#if defined(LCUE_TESSELLATION_ENABLED)
	tessellationEnabled = (m_activeShaderStages == Gnm::kActiveShaderStagesLsHsVsPs) | (m_activeShaderStages == Gnm::kActiveShaderStagesLsHsEsGsVsPs);
	if (tessellationEnabled)
		preDrawTessellation(geometryShaderEnabled);
#endif
	
	const Gnmx::VsShader* vsShader = ((const Gnmx::VsShader*)m_boundShader[Gnm::kShaderStageVs]);
	const Gnmx::PsShader* psShader = ((const Gnmx::PsShader*)m_boundShader[Gnm::kShaderStagePs]);

	if (m_dirtyShader[Gnm::kShaderStageVs])
		m_dcb->setVsShader( &vsShader->m_vsStageRegisters, m_boundShaderModifier[Gnm::kShaderStageVs] );
	if (m_dirtyShader[Gnm::kShaderStagePs])
		m_dcb->setPsShader( (psShader != NULL)? &psShader->m_psStageRegisters : NULL );
	
	// Handle Immediate Constant Buffer on CB slot 15
	if (m_dirtyShader[Gnm::kShaderStageVs] || m_dirtyShaderResources[Gnm::kShaderStageVs])
		LCUE_IMMEDIATE_CB_UPDATE_ON_GRAPHICS(Gnm::kShaderStageVs, vsShader);
	if (m_dirtyShader[Gnm::kShaderStagePs] || m_dirtyShaderResources[Gnm::kShaderStagePs])
		LCUE_IMMEDIATE_CB_UPDATE_ON_GRAPHICS(Gnm::kShaderStagePs, psShader);

	if (m_dirtyShaderResources[Gnm::kShaderStageVs])
	{
		// Validate after handling immediate CBs (if necessary) and before flushing scratch buffer
		LCUE_VALIDATE_RESOURCE_CHECK_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStageVs] );

		const uint32_t* flushAddress = flushScratchBuffer(Gnm::kShaderStageVs);
		updateCommonPtrsInUserDataSgprs(Gnm::kShaderStageVs, flushAddress);
		updateLsEsVsPtrsInUserDataSgprs(Gnm::kShaderStageVs, flushAddress);
	}
	if (m_dirtyShaderResources[Gnm::kShaderStagePs])
	{
		// Validate after handling immediate CBs (if necessary) and before flushing scratch buffer
		LCUE_VALIDATE_RESOURCE_CHECK_TABLE(	&m_boundShaderResourcesValidation[Gnm::kShaderStagePs] );

		const uint32_t* flushAddress = flushScratchBuffer(Gnm::kShaderStagePs);
		updateCommonPtrsInUserDataSgprs(Gnm::kShaderStagePs, flushAddress);
	}

// This can be enabled to avoid regenerating and resetting the pixel usage table everytime
#if !defined(LCUE_SKIP_VS_PS_SEMANTIC_TABLE)
	if (m_dirtyShader[Gnm::kShaderStageVs] || m_dirtyShader[Gnm::kShaderStagePs])
	{
		uint32_t psInputs[32];
		if (psShader != NULL && psShader->m_numInputSemantics != 0)
		{
			Gnm::generatePsShaderUsageTable(psInputs,
				vsShader->getExportSemanticTable(), vsShader->m_numExportSemantics,
				psShader->getPixelInputSemanticTable(), psShader->m_numInputSemantics);
			m_dcb->setPsShaderUsage(psInputs, psShader->m_numInputSemantics);
		}
	}
#endif

	__builtin_memset(&m_dirtyShader[Gnm::kShaderStagePs], 0, (Gnm::kShaderStageCount-1) * sizeof(bool));
	__builtin_memset(&m_dirtyShaderResources[Gnm::kShaderStagePs], 0, (Gnm::kShaderStageCount-1) * sizeof(bool));
	LCUE_PROFILE_DRAW_CALL;
}


#if defined LCUE_GEOMETRY_SHADERS_ENABLED
void LCUE::GraphicsCUE::setEsShader(const sce::Gnmx::EsShader* shader, uint32_t shaderModifier, const void* fetchShader, const ShaderResourceOffsets* table)
{
	LCUE_ASSERT( shader != NULL && table != NULL && table->shaderStage == Gnm::kShaderStageEs);
	LCUE_VALIDATE_RESOURCE_INIT_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStageEs], table );

	m_dirtyShaderResources[Gnm::kShaderStageEs] = true;
	m_dirtyShader[Gnm::kShaderStageEs] |= (m_boundShader[Gnm::kShaderStageEs] != shader);

	m_boundShaderResourceOffsets[Gnm::kShaderStageEs] = table;
	m_boundShader[Gnm::kShaderStageEs] = shader;
	m_boundFetchShader[Gnm::kShaderStageEs] = fetchShader;
	m_boundShaderModifier[Gnm::kShaderStageEs] = shaderModifier;
}


void LCUE::GraphicsCUE::setEsFetchShader(uint32_t shaderModifier, const void* fetchShader)
{
	m_boundShaderModifier[Gnm::kShaderStageEs] = shaderModifier;
	m_boundFetchShader[Gnm::kShaderStageEs] = fetchShader;
}


void LCUE::GraphicsCUE::setGsVsShaders(const sce::Gnmx::GsShader* shader, const ShaderResourceOffsets* gsTable)
{
	LCUE_ASSERT( shader != NULL && gsTable != NULL && gsTable->shaderStage == Gnm::kShaderStageGs);
	LCUE_VALIDATE_RESOURCE_INIT_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStageGs], gsTable );

	m_dirtyShaderResources[Gnm::kShaderStageGs] = true;
	m_dirtyShader[Gnm::kShaderStageGs] |= (m_boundShader[Gnm::kShaderStageGs] != shader);
	m_boundShaderResourceOffsets[Gnm::kShaderStageGs] = gsTable;
	m_boundShader[Gnm::kShaderStageGs] = shader;

	const sce::Gnmx::VsShader* vsShader = shader->getCopyShader();
	const ShaderResourceOffsets* vsShaderResourceOffsets = (vsShader->m_common.m_numInputUsageSlots == 1)? &m_fixedGsVsShaderResourceOffsets : 
		&m_fixedGsVsStreamOutShaderResourceOffsets;
	setVsShader(vsShader, vsShaderResourceOffsets);
}


void LCUE::GraphicsCUE::setStreamoutBuffers(int32_t startApiSlot, int32_t apiSlotCount, const sce::Gnm::Buffer* restrict buffer)
{
	LCUE_ASSERT(startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxStreamOutBufferCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets[(int32_t)Gnm::kShaderStageVs] != NULL && buffer != NULL && buffer->isBuffer());
	//LCUE_VALIDATE_RESOURCE_MEMORY_MAPPED(buffer, SCE_KERNEL_PROT_GPU_READ|SCE_KERNEL_PROT_GPU_WRITE);
	// TODOBE There's a special validation for streamout buffers

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)Gnm::kShaderStageVs];
	for (int32_t i=0; i<apiSlotCount; ++i)
	{
		int32_t currentApiSlot = startApiSlot+i;

		// TODO No info available, stream-outs are in the VS but there's footer for the VS in a VsGs shader (only GS has a footer).
		//LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets[(int32_t)Gnm::kShaderStageVs]->streamOutDwOffset[currentApiSlot] != 0xFFFF);
		//LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation[(int32_t)Gnm::kShaderStageVs].streamOutIsBound[currentApiSlot]);

		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets[(int32_t)Gnm::kShaderStageVs]->streamOutPtrSgpr != 0xFF);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, Gnm::kShaderStageVs, table->streamOutDwOffset[currentApiSlot], buffer+i, sizeof(Gnm::Buffer));
		LCUE_PROFILE_SET_VSHARP_CALL;
	}

	LCUE_PROFILE_SET_SOBUFFERS_CALL;
	m_dirtyShaderResources[(int32_t)Gnm::kShaderStageVs] = true;
}
#endif


#if defined LCUE_TESSELLATION_ENABLED
void LCUE::GraphicsCUE::setLsShader(const Gnmx::LsShader* shader, uint32_t shaderModifier, const void* fetchShader, const ShaderResourceOffsets* table)
{
	LCUE_ASSERT( shader != NULL && table != NULL && table->shaderStage == Gnm::kShaderStageLs);
	LCUE_VALIDATE_RESOURCE_INIT_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStageLs], table );

	m_dirtyShaderResources[Gnm::kShaderStageLs] = true;
	m_dirtyShader[Gnm::kShaderStageLs] |= (m_boundShader[Gnm::kShaderStageLs] != shader);

	m_boundShaderResourceOffsets[Gnm::kShaderStageLs] = table;
	m_boundShader[Gnm::kShaderStageLs] = shader;
	m_boundFetchShader[Gnm::kShaderStageLs] = fetchShader;
	m_boundShaderModifier[Gnm::kShaderStageLs] = shaderModifier;
}


void LCUE::GraphicsCUE::setLsFetchShader(uint32_t shaderModifier, const void* fetchShader)
{
	m_boundShaderModifier[Gnm::kShaderStageLs] = shaderModifier;
	m_boundFetchShader[Gnm::kShaderStageLs] = fetchShader;
}


void LCUE::GraphicsCUE::setHsShader(const Gnmx::HsShader* shader, const ShaderResourceOffsets* table, int32_t optionalTgPatchCount)
{
	LCUE_ASSERT( shader != NULL && table != NULL && table->shaderStage == Gnm::kShaderStageHs);
	LCUE_VALIDATE_RESOURCE_INIT_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStageHs], table );

	m_dirtyShaderResources[Gnm::kShaderStageHs] = true;
	m_dirtyShader[Gnm::kShaderStageHs] |= (m_boundShader[Gnm::kShaderStageHs] != shader);

	m_boundShaderResourceOffsets[Gnm::kShaderStageHs] = table;
	m_boundShader[Gnm::kShaderStageHs] = shader;
	m_tessellationDesiredTgPatchCount = optionalTgPatchCount;
}
#endif


void LCUE::GraphicsCUE::setVsShader(const Gnmx::VsShader* shader, uint32_t shaderModifier, const void* fetchShader, const ShaderResourceOffsets* table)
{
	LCUE_ASSERT( shader != NULL && table != NULL && table->shaderStage == Gnm::kShaderStageVs);
	LCUE_VALIDATE_RESOURCE_INIT_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStageVs], table );

	m_dirtyShaderResources[Gnm::kShaderStageVs] = true;
	m_dirtyShader[Gnm::kShaderStageVs] |= (m_boundShader[Gnm::kShaderStageVs] != shader);

	m_boundShaderResourceOffsets[Gnm::kShaderStageVs] = table;
	m_boundShader[Gnm::kShaderStageVs] = shader;
	m_boundFetchShader[Gnm::kShaderStageVs] = fetchShader;
	m_boundShaderModifier[Gnm::kShaderStageVs] = shaderModifier;
}


void LCUE::GraphicsCUE::setVsFetchShader(uint32_t shaderModifier, const void* fetchShader)
{
	m_boundShaderModifier[Gnm::kShaderStageVs] = shaderModifier;
	m_boundFetchShader[Gnm::kShaderStageVs] = fetchShader;
}


void LCUE::GraphicsCUE::setPsShader(const Gnmx::PsShader* shader, const ShaderResourceOffsets* table)
{
	LCUE_ASSERT( (shader == NULL && table == NULL) || (shader != NULL && table != NULL) );
	LCUE_ASSERT( table == NULL || table->shaderStage == Gnm::kShaderStagePs);
	LCUE_ASSERT( table == NULL || (m_bufferCurrent + table->requiredBufferSizeInDwords) < m_bufferEnd[m_bufferIndex] );
	LCUE_VALIDATE_RESOURCE_INIT_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStagePs], table );

	// Special case: if the Pixel Shader is NULL we don't mark shaderResourceOffsets as dirty to prevent flushing the scratch buffer
	m_dirtyShaderResources[Gnm::kShaderStagePs] = (shader != NULL); 
	m_dirtyShader[Gnm::kShaderStagePs] |= (m_boundShader[Gnm::kShaderStagePs] != shader);

	m_boundShaderResourceOffsets[Gnm::kShaderStagePs] = table;
	m_boundShader[Gnm::kShaderStagePs] = shader;
}


void LCUE::GraphicsCUE::setCsShader(const Gnmx::CsShader* shader, const ShaderResourceOffsets* table)
{
	LCUE_ASSERT( shader != NULL && table != NULL && table->shaderStage == Gnm::kShaderStageCs );
	LCUE_ASSERT( (m_bufferCurrent + table->requiredBufferSizeInDwords) < m_bufferEnd[m_bufferIndex] );
	LCUE_VALIDATE_RESOURCE_INIT_TABLE( &m_boundShaderResourcesValidation[Gnm::kShaderStageCs], table );

	m_dirtyShaderResources[Gnm::kShaderStageCs] = true;
	m_dirtyShader[Gnm::kShaderStageCs] |= (m_boundShader[Gnm::kShaderStageCs] != shader);
	
	m_boundShaderResourceOffsets[Gnm::kShaderStageCs] = table;
	m_boundShader[Gnm::kShaderStageCs] = shader;
}



void LCUE::GraphicsCUE::setAppendConsumeCounterRange(Gnm::ShaderStage shaderStage, uint32_t gdsMemoryBaseInBytes, uint32_t countersSizeInBytes)
{
	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount && m_boundShaderResourceOffsets[(int32_t)shaderStage] != NULL);
	LCUE_ASSERT((gdsMemoryBaseInBytes%4)==0 && gdsMemoryBaseInBytes < UINT16_MAX && (countersSizeInBytes%4)==0 && countersSizeInBytes < UINT16_MAX);

	do
	{
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets[(int32_t)shaderStage]->appendConsumeCounterSgpr != 0xFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation[shaderStage].appendConsumeCounterIsBound);
		m_boundShaderAppendConsumeCounterRange[shaderStage] = (gdsMemoryBaseInBytes << 16) | countersSizeInBytes;
	} while(false);

	LCUE_PROFILE_SET_APPEND_CONSUME_COUNTERS_CALL;
	m_dirtyShaderResources[(int32_t)shaderStage] = true;
}


void LCUE::GraphicsCUE::setConstantBuffers(Gnm::ShaderStage shaderStage, int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Buffer* restrict buffer)
{
	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount && startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxConstantBufferCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets[(int32_t)shaderStage] != NULL && buffer != NULL && buffer->isBuffer());

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_CONSTANT_BUFFER(buffer+i);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets[(int32_t)shaderStage]->constBufferDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation[shaderStage].constBufferOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, shaderStage, table->constBufferDwOffset[currentApiSlot], buffer+i, sizeof(Gnm::Buffer));
		LCUE_PROFILE_SET_VSHARP_CALL;
	}

	LCUE_PROFILE_SET_CBUFFERS_CALL;
	m_dirtyShaderResources[(int32_t)shaderStage] = true;
}


void LCUE::GraphicsCUE::setVertexBuffers(Gnm::ShaderStage shaderStage, int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Buffer* restrict buffer)
{
	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount && startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxVertexBufferCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets[(int32_t)shaderStage] != NULL && buffer != NULL && buffer->isBuffer());

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];
	for (int32_t i=0; i<apiSlotCount; ++i)
	{
		LCUE_VALIDATE_VERTEX_BUFFER(buffer+i);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets[(int32_t)shaderStage]->vertexBufferDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation[shaderStage].vertexBufferOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, shaderStage, table->vertexBufferDwOffset[currentApiSlot], buffer+i, sizeof(Gnm::Buffer));
		LCUE_PROFILE_SET_VSHARP_CALL;
	}

	LCUE_PROFILE_SET_VBUFFERS_CALL;
	m_dirtyShaderResources[(int32_t)shaderStage] = true;
}


void LCUE::GraphicsCUE::setBuffers(Gnm::ShaderStage shaderStage, int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Buffer* restrict buffer)
{
	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount && startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxResourceCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets[(int32_t)shaderStage] != NULL && buffer != NULL && buffer->isBuffer());

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_BUFFER(buffer+i);
		// Cannot currently be validated as this information is not stored for entries in the resource-flat-table (possible to retrieve from Shader::Binary)
		//LCUE_ASSERT((m_boundShaderResourceOffsets[(int32_t)shaderStage]->resourceDwOffset[currentApiSlot] & kResourceIsVSharp) != 0);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets[(int32_t)shaderStage]->resourceDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation[shaderStage].resourceOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, shaderStage, table->resourceDwOffset[currentApiSlot], buffer+i, sizeof(Gnm::Buffer));
		LCUE_PROFILE_SET_VSHARP_CALL;
	}

	LCUE_PROFILE_SET_BUFFERS_CALL;
	m_dirtyShaderResources[(int32_t)shaderStage] = true;
}


void LCUE::GraphicsCUE::setRwBuffers(Gnm::ShaderStage shaderStage, int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Buffer* restrict buffer)
{
	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount && startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxRwResourceCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets[(int32_t)shaderStage] != NULL && buffer != NULL && buffer->isBuffer());
	LCUE_ASSERT(buffer->getResourceMemoryType() != Gnm::kResourceMemoryTypeRO);

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_RWBUFFER(buffer+i);
		// Cannot currently be validated as this information is not stored for entries in the resource-flat-table (possible to retrieve from Shader::Binary)
		//LCUE_ASSERT((m_boundShaderResourceOffsets[(int32_t)shaderStage]->rwResourceDwOffset[currentApiSlot] & kResourceIsVSharp) != 0);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets[(int32_t)shaderStage]->rwResourceDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation[shaderStage].rwResourceOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, shaderStage, table->rwResourceDwOffset[currentApiSlot], buffer+i, sizeof(Gnm::Buffer));
		LCUE_PROFILE_SET_VSHARP_CALL;
	}

	LCUE_PROFILE_SET_RWBUFFERS_CALL;
	m_dirtyShaderResources[(int32_t)shaderStage] = true;
}


void LCUE::GraphicsCUE::setTextures(Gnm::ShaderStage shaderStage, int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Texture* restrict texture)
{
	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount && startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxResourceCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets[(int32_t)shaderStage] != NULL && texture != NULL && texture->isTexture());

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_TEXTURE(texture+i);
		// Cannot currently be validated as this information is not stored for entries in the resource-flat-table (possible to retrieve from Shader::Binary)
		//LCUE_ASSERT((m_boundShaderResourceOffsets[(int32_t)shaderStage]->resourceDwOffset[currentApiSlot] & kResourceIsVSharp) == 0);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets[(int32_t)shaderStage]->resourceDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation[shaderStage].resourceOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, shaderStage, table->resourceDwOffset[currentApiSlot], texture+i, sizeof(Gnm::Texture));
		LCUE_PROFILE_SET_TSHARP_CALL;
	}

	LCUE_PROFILE_SET_TEXTURES_CALL;
	m_dirtyShaderResources[(int32_t)shaderStage] = true;
}


void LCUE::GraphicsCUE::setRwTextures(Gnm::ShaderStage shaderStage, int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Texture* restrict texture)
{
	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount && startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxRwResourceCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets[(int32_t)shaderStage] != NULL && texture != NULL && texture->isTexture());
	LCUE_ASSERT(texture->getResourceMemoryType() != Gnm::kResourceMemoryTypeRO);

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_RWTEXTURE(texture+i);
		// Cannot currently be validated as this information is not stored for entries in the resource-flat-table (possible to retrieve from Shader::Binary)
		//LCUE_ASSERT((m_boundShaderResourceOffsets[(int32_t)shaderStage]->rwResourceDwOffset[currentApiSlot] & kResourceIsVSharp) == 0);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets[(int32_t)shaderStage]->rwResourceDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation[shaderStage].rwResourceOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, shaderStage, table->rwResourceDwOffset[currentApiSlot], texture+i, sizeof(Gnm::Texture));
		LCUE_PROFILE_SET_TSHARP_CALL;
	}

	LCUE_PROFILE_SET_RWTEXTURES_CALL;
	m_dirtyShaderResources[(int32_t)shaderStage] = true;
}


void LCUE::GraphicsCUE::setSamplers(Gnm::ShaderStage shaderStage, int32_t startApiSlot, int32_t apiSlotCount, const Gnm::Sampler* restrict sampler)
{
	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount && startApiSlot >= 0 && (startApiSlot+apiSlotCount) <= kMaxSamplerCount);
	LCUE_ASSERT(m_boundShaderResourceOffsets[(int32_t)shaderStage] != NULL && sampler != NULL);

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];
	for (int32_t i=0; i<apiSlotCount; i++)
	{
		LCUE_VALIDATE_SAMPLER(sampler+i);

		int32_t currentApiSlot = startApiSlot+i;
		LCUE_ASSERT_OR_CONTINUE(m_boundShaderResourceOffsets[(int32_t)shaderStage]->samplerDwOffset[currentApiSlot] != 0xFFFF);
		LCUE_VALIDATE_RESOURCE_MARK_USED(m_boundShaderResourcesValidation[shaderStage].samplerOffsetIsBound[currentApiSlot]);
		setDataInUserDataSgprOrMemory(m_dcb, m_scratchBuffer, shaderStage, table->samplerDwOffset[currentApiSlot], sampler+i, sizeof(Gnm::Sampler));
		LCUE_PROFILE_SET_SSHARP_CALL;
	}

	LCUE_PROFILE_SET_SAMPLERS_CALL;
	m_dirtyShaderResources[(int32_t)shaderStage] = true;
}


void LCUE::GraphicsCUE::setUserData(sce::Gnm::ShaderStage shaderStage, int32_t startSgpr, int32_t sgprCount, const uint32_t* data)
{
	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount && startSgpr >= 0 && (startSgpr+sgprCount) <= kMaxUserDataCount);

	setPersistentRegisterRange(m_dcb, shaderStage, startSgpr, data, sgprCount);
	LCUE_PROFILE_SET_USER_DATA_CALL;
}


void LCUE::GraphicsCUE::setPtrInUserData(sce::Gnm::ShaderStage shaderStage, int32_t startSgpr, const void* gpuAddress)
{
	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount && startSgpr >= 0 && (startSgpr) <= kMaxUserDataCount);
	setPtrInPersistentRegister(m_dcb, shaderStage, startSgpr, gpuAddress);
	LCUE_PROFILE_SET_PTR_USER_DATA_CALL;
}


void LCUE::GraphicsCUE::setUserSrtBuffer(sce::Gnm::ShaderStage shaderStage, const void* buffer, uint32_t sizeInDwords)
{
	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount && (sizeInDwords > 0 && sizeInDwords <= kMaxSrtUserDataCount));
	LCUE_ASSERT(m_boundShaderResourceOffsets[(int32_t)shaderStage] != NULL && buffer != NULL);

	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];
	LCUE_ASSERT(sizeInDwords == table->userSrtDataCount);
	setUserData(shaderStage, table->userSrtDataSgpr, table->userSrtDataCount, (const uint32_t *)buffer);
	LCUE_PROFILE_SET_SRT_USER_DATA_CALL;
	LCUE_PROFILE_SET_INTERNAL_SRT_USER_DATA_CALL;
}


void LCUE::GraphicsCUE::setInternalSrtBuffer(sce::Gnm::ShaderStage shaderStage, const void* buffer)
{
	LCUE_ASSERT(false); // Not supported, the LCUE already binds directly to the EUD via the scratch buffer

// 	LCUE_ASSERT(shaderStage < Gnm::kShaderStageCount);
// 	LCUE_ASSERT(m_boundShaderResourceOffsets[(int32_t)shaderStage] != NULL && buffer != NULL);
// 
// 	const ShaderResourceOffsets* table = m_boundShaderResourceOffsets[(int32_t)shaderStage];
// 	setPtrInUserData(shaderStage, table->userInternalSrtDataPtrSgpr, buffer);
// 	LCUE_PROFILE_SET_INTERNAL_SRT_USER_DATA_CALL;
}


void LCUE::ComputeContext::init(uint32_t* dcbBuffer, int32_t dcbBufferSizeInDwords,
								uint32_t* resourceBufferInGarlic, int32_t resourceBufferSizeInDwords, 
								uint32_t* globalInternalResourceTableInGarlic,
								sce::Gnm::CommandCallbackFunc callbackFunc, void *callbackUserData)
{
	LCUE_ASSERT(dcbBuffer != NULL);
	LCUE_ASSERT(dcbBufferSizeInDwords > 0 && (dcbBufferSizeInDwords * sizeof(uint32_t)) <= Gnm::kIndirectBufferMaximumSizeInBytes);

	m_dcb.init(dcbBuffer, dcbBufferSizeInDwords * sizeof(uint32_t), callbackFunc, callbackUserData);

	ComputeCUE::init(&resourceBufferInGarlic, 1, resourceBufferSizeInDwords, globalInternalResourceTableInGarlic);
	ComputeCUE::setDcb(&m_dcb);
}


void LCUE::GraphicsContext::init(uint32_t* dcbBuffer, int32_t dcbBufferSizeInDwords, uint32_t* resourceBufferInGarlic, int32_t resourceBufferSizeInDwords, 
	uint32_t* globalInternalResourceTableInGarlic, Gnm::CommandCallbackFunc callbackFunc, void* callbackUserData)
{
	LCUE_ASSERT(dcbBuffer != NULL);
	LCUE_ASSERT(dcbBufferSizeInDwords > 0 && (dcbBufferSizeInDwords * sizeof(uint32_t)) <= Gnm::kIndirectBufferMaximumSizeInBytes);

	m_dcb.init(dcbBuffer, dcbBufferSizeInDwords * sizeof(uint32_t), callbackFunc, callbackUserData);

	GraphicsCUE::init(&resourceBufferInGarlic, 1, resourceBufferSizeInDwords, globalInternalResourceTableInGarlic);
	GraphicsCUE::setDcb(&m_dcb);
}


void LCUE::GraphicsContext::setEsGsRingBuffer(void* ringBufferInGarlic, uint32_t ringSizeInBytes, uint32_t maxExportVertexSizeInDword)
{
	Gnm::Buffer ringReadDescriptor;
	Gnm::Buffer ringWriteDescriptor;
	ringReadDescriptor.initAsEsGsReadDescriptor(ringBufferInGarlic, ringSizeInBytes);
	ringWriteDescriptor.initAsEsGsWriteDescriptor(ringBufferInGarlic, ringSizeInBytes);

	setGlobalInternalResource(Gnm::kShaderGlobalResourceEsGsReadDescriptor, &ringReadDescriptor);
	setGlobalInternalResource(Gnm::kShaderGlobalResourceEsGsWriteDescriptor, &ringWriteDescriptor);

	m_dcb.setupEsGsRingRegisters(maxExportVertexSizeInDword);
}


void LCUE::GraphicsContext::setGsVsRingBuffers(void* ringBufferInGarlic, uint32_t ringSizeInBytes, const uint32_t vertexSizePerStreamInDword[4], uint32_t maxOutputVertexCount)
{
	Gnm::Buffer ringReadDescriptor;
	Gnm::Buffer ringWriteDescriptor[4];

	ringReadDescriptor.initAsGsVsReadDescriptor(ringBufferInGarlic, ringSizeInBytes);
	ringWriteDescriptor[0].initAsGsVsWriteDescriptor(ringBufferInGarlic, 0, vertexSizePerStreamInDword, maxOutputVertexCount);
	ringWriteDescriptor[1].initAsGsVsWriteDescriptor(ringBufferInGarlic, 1, vertexSizePerStreamInDword, maxOutputVertexCount);
	ringWriteDescriptor[2].initAsGsVsWriteDescriptor(ringBufferInGarlic, 2, vertexSizePerStreamInDword, maxOutputVertexCount);
	ringWriteDescriptor[3].initAsGsVsWriteDescriptor(ringBufferInGarlic, 3, vertexSizePerStreamInDword, maxOutputVertexCount);

	setGlobalInternalResource(Gnm::kShaderGlobalResourceGsVsReadDescriptor, &ringReadDescriptor);
	setGlobalInternalResource(Gnm::kShaderGlobalResourceGsVsWriteDescriptor0, 4, &ringWriteDescriptor[0]);

	m_dcb.setupGsVsRingRegisters(vertexSizePerStreamInDword, maxOutputVertexCount);
}


LCUE_FORCE_INLINE 
	void LCUE::computeTessellationTgPatchCountAndLdsSize(int32_t* outTgPatchCount, int32_t* outTgLdsSizeInBytes, const Gnmx::LsShader* localShader, const Gnmx::HsShader* hullShader,
	int32_t optionalDesiredTgPatchCount)
{
	LCUE_ASSERT(outTgPatchCount != NULL);
	LCUE_ASSERT(outTgLdsSizeInBytes != NULL);
	LCUE_ASSERT(localShader != NULL);
	LCUE_ASSERT(hullShader != NULL);

	const Gnm::HullStateConstants* hsConstants = &hullShader->m_hullStateConstants;
	uint32_t lsStrideInBytes = localShader->m_lsStride;

	// Check if the patchCount provided is valid
	const int32_t kLdsSizeAlignmentInBytes = 512;
	const int32_t kSimdPerCuCount = 4;
	const int32_t kVgprsPerSimd = 256;
	const int32_t kCuLdsSizeInBytes = 32 * 1024; // TODO: It's 64KB but it appears a TG is still limited somewhere to 32KB
	const int32_t kMaxThreadsPerTg = 1024; // TODO: It should be 2048 but for some reason it's 1024

	int32_t localRequiredVgpr = (localShader->m_lsStageRegisters.m_spiShaderPgmRsrc1Ls & 0x3F) * 4 + 4;
	int32_t hullRequiredVgpr = (hullShader->m_hsStageRegisters.m_spiShaderPgmRsrc1Hs & 0x3F) * 4 + 4;
	LCUE_ASSERT(localRequiredVgpr%4==0 && hullRequiredVgpr%4==0);

	int32_t patchSizeInBytes = hsConstants->m_numInputCP * lsStrideInBytes + 
		hsConstants->m_numOutputCP * hsConstants->m_cpStride + 
		hsConstants->m_numPatchConst * 16;
	//hsConstants->m_tessFactorStride; // TODO: This is probably the correct way of doing it but I'm doing it the wrong way to match TessellationDataConstantBuffer::init

	int32_t localMaxSimdThreadsLimitedByVgpr = (kVgprsPerSimd / localRequiredVgpr);
	int32_t localMaxPatchesLimitedByVgpr = (kSimdPerCuCount * localMaxSimdThreadsLimitedByVgpr) / hsConstants->m_numInputCP;
	int32_t hullMaxSimdThreadsLimitedByVgpr = (kVgprsPerSimd / hullRequiredVgpr);
	int32_t hullMaxPatchesLimitedByVgpr = (kSimdPerCuCount * hullMaxSimdThreadsLimitedByVgpr) / hsConstants->m_numThreads;
	int32_t maxTgPatchesLimitedByVgpr = SCE_GNM_MIN(localMaxPatchesLimitedByVgpr, hullMaxPatchesLimitedByVgpr);

	int32_t localMaxPatchesLimitedByThreads = kMaxThreadsPerTg / hsConstants->m_numInputCP;
	int32_t hullMaxPatchesLimitedByThreads = kMaxThreadsPerTg / hsConstants->m_numThreads;
	int32_t maxTgPatchesLimitedByThreads = SCE_GNM_MIN(localMaxPatchesLimitedByThreads, hullMaxPatchesLimitedByThreads);

	int32_t maxTgPatchesLimitedByLds = kCuLdsSizeInBytes / patchSizeInBytes;
	int32_t maxTgPatches = SCE_GNM_MIN( SCE_GNM_MIN(maxTgPatchesLimitedByVgpr, maxTgPatchesLimitedByThreads), maxTgPatchesLimitedByLds);
	maxTgPatches = SCE_GNM_MIN(maxTgPatches, optionalDesiredTgPatchCount);

	// Update required LDS size
	int32_t ldsRequiredSizeInBytes = ((patchSizeInBytes * maxTgPatches) + kLdsSizeAlignmentInBytes - 1) & ~(kLdsSizeAlignmentInBytes-1);
	LCUE_ASSERT(ldsRequiredSizeInBytes <= kCuLdsSizeInBytes);

	*outTgPatchCount = maxTgPatches;
	*outTgLdsSizeInBytes = ldsRequiredSizeInBytes;
}


int32_t getUsedApiSlotsFromMask(int32_t* outUsedApiSlots, int32_t usedApiSlotsCount, uint32_t mask[4], int32_t maxResourceCount)
{
	int32_t resourceCount = 0;
	for (int32_t slot = 0; slot < usedApiSlotsCount; ++slot)
		if (mask[slot>>5] & (1<<(slot & 0x1F)))
			outUsedApiSlots[resourceCount++] = slot;

#if defined LCUE_VALIDATE_COMPLETE_RESOURCE_BINDING_ENABLED
	// Check if the shader uses any API-slot over the maximum count configured to be handled by the LCUE
	for (int32_t slot = usedApiSlotsCount; slot < maxResourceCount; ++slot)
		LCUE_ASSERT( (mask[slot>>5] & (1<<(slot & 0x1F))) == 0);
#endif

	return resourceCount;
}


int32_t getUsedApiSlotsFromMask(int32_t* outUsedApiSlots, int32_t usedApiSlotsCount, uint32_t mask, int32_t maxResourceCount)
{
	int32_t resourceCount = 0;
	for (int32_t slot = 0; slot < usedApiSlotsCount; ++slot)
		if ( ((1<<slot) & mask))
			outUsedApiSlots[resourceCount++] = slot;

#if defined LCUE_VALIDATE_COMPLETE_RESOURCE_BINDING_ENABLED
	// Check if the shader uses any API-slot over the maximum count configured to be handled by the LCUE
	for (int32_t slot = usedApiSlotsCount; slot < maxResourceCount; ++slot)
		LCUE_ASSERT( ((1<<slot) & mask) == 0);
#endif

	return resourceCount;
}


int32_t LCUE::patchSROTableWithSemanticTable(ShaderResourceOffsets* outTable, const ShaderResourceOffsets* table, const int32_t* semanticRemapTable, int32_t semanticRemapTableSizeInItems)
{
	LCUE_ASSERT(outTable != NULL && table != NULL);
	LCUE_ASSERT(semanticRemapTable != NULL && semanticRemapTableSizeInItems >= 0 && semanticRemapTableSizeInItems < kMaxVertexBufferCount);

	// Validate remap table
#if defined(LCUE_DEBUG)
	bool remapUsed[LCUE::kMaxVertexBufferCount];
	memset(remapUsed, 0, LCUE::kMaxVertexBufferCount * sizeof(bool));
	for (int32_t i=0; i<semanticRemapTableSizeInItems; ++i)
	{
		int32_t remapIndex = semanticRemapTable[i];
		if (remapIndex == -1)
			continue;

		LCUE_ASSERT(!remapUsed[remapIndex]);
		remapUsed[remapIndex] = true;
	}
#endif

	ShaderResourceOffsets resultTable;
	memcpy(&resultTable, table, sizeof(ShaderResourceOffsets));

	// Patch
	int32_t remapCountDiff = 0;
	for (int32_t i=0; i<semanticRemapTableSizeInItems; ++i)
	{
		int32_t remapIndex = semanticRemapTable[i];
		if (remapIndex == -1)
		{
			// Decrease VB input count
			if (table->vertexBufferDwOffset[i] != 0xFFFF)
				remapCountDiff--;

			resultTable.vertexBufferDwOffset[i] = 0xFFFF;
		}
		else
		{
			// Increased VB input count
			if (table->vertexBufferDwOffset[i] == 0xFFFF)
				remapCountDiff++;

			// Make sure it remaps to a valid VB
			LCUE_ASSERT(table->vertexBufferDwOffset[remapIndex] != 0xFFFF);
			resultTable.vertexBufferDwOffset[i] = table->vertexBufferDwOffset[remapIndex];
		}
	}

	// If it's not zero you increased the number of input-VBs
	LCUE_ASSERT(remapCountDiff == 0);
	memcpy(outTable, &resultTable, sizeof(ShaderResourceOffsets));

	return remapCountDiff;
}


int32_t LCUE::generateSROTable(ShaderResourceOffsets* outTable, Gnm::ShaderStage shaderStage, const void* gnmxShaderStruct)
{
	// Get the shader pointer and its size from the GNMX shader type
	const Gnmx::ShaderCommonData* shaderCommonData = (const Gnmx::ShaderCommonData*)gnmxShaderStruct;
	const uint32_t* shaderRegisters = (const uint32_t*)(shaderCommonData+1);

	const void* shaderCode = (void*)( ((uintptr_t)shaderRegisters[0] << 8ULL) | ((uintptr_t)shaderRegisters[1] << 40ULL) );
	int32_t shaderCodeSize = ((const Gnmx::ShaderCommonData*)gnmxShaderStruct)->m_shaderSize;
	LCUE_ASSERT(shaderCode != NULL && shaderCodeSize > 0);

	return generateSROTable(outTable, shaderStage, gnmxShaderStruct, shaderCode, shaderCodeSize, shaderCommonData->m_shaderIsUsingSrt);
}


int32_t LCUE::generateSROTable(ShaderResourceOffsets* outTable, Gnm::ShaderStage shaderStage, const void* gnmxShaderStruct, const void* shaderCode, int32_t shaderCodeSizeInBytes, bool isSrtUsed)
{
	LCUE_ASSERT(outTable != NULL);
	LCUE_ASSERT(shaderStage <= Gnm::kShaderStageCount);

	// Shader footer. There's a footer after the shader code which contains resource info used populate ShaderResourceOffsets
	ShaderBinaryInfo const *shaderBinaryInfo = (ShaderBinaryInfo const*)((uintptr_t)shaderCode + shaderCodeSizeInBytes - sizeof(ShaderBinaryInfo));
	LCUE_ASSERT( (*((uint64_t const*)shaderBinaryInfo->m_signature) & kShaderBinaryInfoSignatureMask) == kShaderBinaryInfoSignatureU64);
	
	// Get usage masks and input usage slots
	uint32_t const* usageMasks = (unsigned int const*)((unsigned char const*)shaderBinaryInfo - shaderBinaryInfo->m_chunkUsageBaseOffsetInDW*4);
	int32_t inputUsageSlotsCount = shaderBinaryInfo->m_numInputUsageSlots;
	Gnm::InputUsageSlot const* inputUsageSlots = (Gnm::InputUsageSlot const*)usageMasks - inputUsageSlotsCount;
	 
	// Cache shader input information into the ShaderResource Offsets table
	__builtin_memset(outTable, 0xFF, sizeof(ShaderResourceOffsets));
	outTable->shaderStage = shaderStage;
	outTable->isSrtShader = isSrtUsed;
	int32_t lastUserDataResourceSizeInDwords = 0;
	int32_t requiredMemorySizeInDwords = 0;

	// Here we handle all immediate resources s[1:16] plus s[16:48] (extended user data)
	// Resources that go into the extended user data also have "immediate" usage type, although they are stored in a table (not fetch by the SPI)
	for (int32_t i=0; i<inputUsageSlotsCount; ++i)
	{
		int32_t apiSlot = inputUsageSlots[i].m_apiSlot;
		int32_t startRegister = inputUsageSlots[i].m_startRegister;
		bool isVSharp = (inputUsageSlots[i].m_resourceType == 0);
		uint16_t vsharpFlag = (isVSharp)? kResourceIsVSharp : 0;

		uint16_t extendedRegisterOffsetInDwords = (startRegister >= 16)? (startRegister-16) : 0;
		requiredMemorySizeInDwords = (requiredMemorySizeInDwords > extendedRegisterOffsetInDwords)?
			requiredMemorySizeInDwords : extendedRegisterOffsetInDwords;

		// Handle immediate resources, including some pointer types
		switch (inputUsageSlots[i].m_usageType)
		{
		case Gnm::kShaderInputUsageImmGdsCounterRange:
			outTable->appendConsumeCounterSgpr = startRegister;
			break;

		//case Gnm::kShaderInputUsagePtrSoBufferTable: // Only present in the VS copy-shader that doesn't have a footer
		//	outTable->streamOutPtrSgpr = startRegister;
		//	break;

		case Gnm::kShaderInputUsageSubPtrFetchShader:
			LCUE_ASSERT(inputUsageSlots[i].m_apiSlot == 0);
			outTable->fetchShaderPtrSgpr = startRegister;
			break;

		case Gnm::kShaderInputUsagePtrInternalGlobalTable:
			LCUE_ASSERT(inputUsageSlots[i].m_apiSlot == 0);
			outTable->globalInternalPtrSgpr = startRegister;
			break;

		case Gnm::kShaderInputUsagePtrExtendedUserData:
			LCUE_ASSERT(inputUsageSlots[i].m_apiSlot == 1);
			// Commented out on purpose, see not supporting setInternalSrtBuffer as LCUE already binds to the EUD via the scratch buffer
// 			if(outTable->isSrtShader)
// 			{
// 				outTable->userInternalSrtDataPtrSgpr = startRegister;
// 			}
// 			else
// 			{
				outTable->userExtendedData1PtrSgpr = startRegister;
// 			}
			break;

		// below resources can either be inside UserData or the EUD
		case Gnm::kShaderInputUsageImmResource:
			LCUE_ASSERT(apiSlot >= 0 && apiSlot < kMaxResourceCount);
			outTable->resourceDwOffset[apiSlot] = (startRegister < 16)?
				(kResourceInUserDataSgpr | vsharpFlag | startRegister) : (vsharpFlag | extendedRegisterOffsetInDwords);
			lastUserDataResourceSizeInDwords = (startRegister < 16)? 0 : 8;
			break;

		case Gnm::kShaderInputUsageImmRwResource:
			LCUE_ASSERT(apiSlot >= 0 && apiSlot < kMaxRwResourceCount);
			outTable->rwResourceDwOffset[apiSlot] = (inputUsageSlots[i].m_startRegister < 16)?
				(kResourceInUserDataSgpr | vsharpFlag | startRegister) : (vsharpFlag | extendedRegisterOffsetInDwords);
			lastUserDataResourceSizeInDwords = (startRegister < 16)? 0 : 8;
			break;

		case Gnm::kShaderInputUsageImmSampler:
			LCUE_ASSERT(apiSlot >= 0 && apiSlot < kMaxSamplerCount);
			outTable->samplerDwOffset[apiSlot] = (inputUsageSlots[i].m_startRegister < 16)?
				(kResourceInUserDataSgpr | startRegister) : extendedRegisterOffsetInDwords;
			lastUserDataResourceSizeInDwords = (startRegister < 16)? 0 : 4;
			break;

		case Gnm::kShaderInputUsageImmConstBuffer:
			LCUE_ASSERT(apiSlot >= 0 && apiSlot < kMaxConstantBufferCount);
			outTable->constBufferDwOffset[apiSlot] = (inputUsageSlots[i].m_startRegister < 16)?
				(kResourceInUserDataSgpr | startRegister) : extendedRegisterOffsetInDwords;
			lastUserDataResourceSizeInDwords = (startRegister < 16)? 0 : 4;
			break;

		case Gnm::kShaderInputUsageImmVertexBuffer:
			LCUE_ASSERT(apiSlot >= 0 && apiSlot < kMaxVertexBufferCount);
			outTable->vertexBufferDwOffset[apiSlot] = (inputUsageSlots[i].m_startRegister < 16)?
				(kResourceInUserDataSgpr | startRegister) : extendedRegisterOffsetInDwords;
			lastUserDataResourceSizeInDwords = (startRegister < 16)? 0 : 4;
			break;
		
		// SRTs will always reside inside the Imm UserData (dwords 0-15), as opposed to the 
		// above resources which can exist in the EUD
		case Gnm::kShaderInputUsageImmShaderResourceTable:
			LCUE_ASSERT(apiSlot >= 0 && apiSlot < kMaxUserDataCount);
			outTable->userSrtDataSgpr = inputUsageSlots[i].m_startRegister;
			outTable->userSrtDataCount = inputUsageSlots[i].m_srtSizeInDWordMinusOne+1;
			break;
		}
	}

	// Make sure we can fit a T# (if required) in the last userOffset
	requiredMemorySizeInDwords += lastUserDataResourceSizeInDwords;

	// Now handle only pointers to resource-tables. Items handled below cannot be found more than once
	for (int32_t i=0; i<inputUsageSlotsCount; ++i)
	{
		uint8_t maskChunks = inputUsageSlots[i].m_chunkMask;
		
		const uint64_t kNibbleToCount = 0x4332322132212110ull;
		uint8_t chunksCount = (kNibbleToCount >> ((maskChunks & 0xF)*4)) & 0xF; (void)chunksCount;
		LCUE_ASSERT(usageMasks+chunksCount <= (uint32_t const*)shaderBinaryInfo);
		
		// Lets fill the resource indices first
		int32_t usedApiSlots[Gnm::kSlotCountResource]; // Use the size of the biggest resource table
		int32_t usedApiSlotCount;

		// This thing will break if there's more than 1 table for any resource type
		uint8_t startRegister = inputUsageSlots[i].m_startRegister;

		switch (inputUsageSlots[i].m_usageType)
		{
		case Gnm::kShaderInputUsagePtrResourceTable:
		{
			LCUE_ASSERT(inputUsageSlots[i].m_apiSlot == 0);
			outTable->resourcePtrSgpr = startRegister;
			outTable->resourceArrayDwOffset = requiredMemorySizeInDwords;

			LCUE_ASSERT(usageMasks < (uint32_t const*)shaderBinaryInfo);
			uint32_t maskArray[4] = { 0, 0, 0, 0};
			if (maskChunks & 1) maskArray[0] = *usageMasks++;
			if (maskChunks & 2) maskArray[1] = *usageMasks++;
			if (maskChunks & 4) maskArray[2] = *usageMasks++;
			if (maskChunks & 8) maskArray[3] = *usageMasks++;
			usedApiSlotCount = getUsedApiSlotsFromMask(&usedApiSlots[0], kMaxResourceCount, maskArray, Gnm::kSlotCountResource);

			int32_t lastUsedApiSlot = usedApiSlots[usedApiSlotCount-1];
			for (uint8_t j=0; j<usedApiSlotCount; j++)
			{
				int32_t currentApiSlot = usedApiSlots[j];
				outTable->resourceDwOffset[currentApiSlot] = requiredMemorySizeInDwords + currentApiSlot * Gnm::kDwordSizeResource;
			}
			requiredMemorySizeInDwords += (lastUsedApiSlot+1) * Gnm::kDwordSizeResource;
		}
		break;

		case Gnm::kShaderInputUsagePtrRwResourceTable:
		{
			LCUE_ASSERT(inputUsageSlots[i].m_apiSlot == 0);
			outTable->rwResourcePtrSgpr = startRegister;
			outTable->rwResourceArrayDwOffset = requiredMemorySizeInDwords;

			LCUE_ASSERT(usageMasks < (uint32_t const*)shaderBinaryInfo);
			usedApiSlotCount = getUsedApiSlotsFromMask(&usedApiSlots[0], kMaxRwResourceCount, *usageMasks++, Gnm::kSlotCountRwResource);

			int32_t lastUsedApiSlot = usedApiSlots[usedApiSlotCount-1];
			for (uint8_t j=0; j<usedApiSlotCount; j++)
			{
				int32_t currentApiSlot = usedApiSlots[j];
				outTable->rwResourceDwOffset[currentApiSlot] = requiredMemorySizeInDwords + currentApiSlot * Gnm::kDwordSizeRwResource;
			}
			requiredMemorySizeInDwords += (lastUsedApiSlot+1) * Gnm::kDwordSizeRwResource;
		}
		break;

		case Gnm::kShaderInputUsagePtrConstBufferTable:
		{
			LCUE_ASSERT(inputUsageSlots[i].m_apiSlot == 0); // TODO BE: Get rid of?
			outTable->constBufferPtrSgpr = startRegister;
			outTable->constBufferArrayDwOffset = requiredMemorySizeInDwords;
			
			LCUE_ASSERT(usageMasks < (uint32_t const*)shaderBinaryInfo);
			usedApiSlotCount = getUsedApiSlotsFromMask(&usedApiSlots[0], kMaxConstantBufferCount, *usageMasks++, Gnm::kSlotCountConstantBuffer);

			int32_t lastUsedApiSlot = usedApiSlots[usedApiSlotCount-1];
			for (uint8_t j=0; j<usedApiSlotCount; j++)
			{
				int32_t currentApiSlot = usedApiSlots[j];
				outTable->constBufferDwOffset[currentApiSlot] = requiredMemorySizeInDwords + currentApiSlot * Gnm::kDwordSizeConstantBuffer;
			}
			requiredMemorySizeInDwords += (lastUsedApiSlot+1) * Gnm::kDwordSizeConstantBuffer;
		}
		break;

		case Gnm::kShaderInputUsagePtrSamplerTable:
		{
			LCUE_ASSERT(inputUsageSlots[i].m_apiSlot == 0);
			outTable->samplerPtrSgpr = startRegister;
			outTable->samplerArrayDwOffset = requiredMemorySizeInDwords;

			LCUE_ASSERT(usageMasks < (uint32_t const*)shaderBinaryInfo);
			usedApiSlotCount = getUsedApiSlotsFromMask(&usedApiSlots[0], kMaxSamplerCount, *usageMasks++, Gnm::kSlotCountSampler);

			int32_t lastUsedApiSlot = usedApiSlots[usedApiSlotCount-1];
			for (uint8_t j=0; j<usedApiSlotCount; j++)
			{
				int32_t currentApiSlot = usedApiSlots[j];
				outTable->samplerDwOffset[currentApiSlot] = requiredMemorySizeInDwords + currentApiSlot * Gnm::kDwordSizeSampler;
			}
			requiredMemorySizeInDwords += (lastUsedApiSlot+1) * Gnm::kDwordSizeSampler;
		}
		break;

		case Gnm::kShaderInputUsagePtrVertexBufferTable:
		{
			LCUE_ASSERT(shaderStage == Gnm::kShaderStageLs || shaderStage == Gnm::kShaderStageEs || shaderStage == Gnm::kShaderStageVs);
			LCUE_ASSERT(inputUsageSlots[i].m_apiSlot == 0);
			outTable->vertexBufferPtrSgpr = startRegister;
			outTable->vertexBufferArrayDwOffset = requiredMemorySizeInDwords;
			
			const Gnm::VertexInputSemantic* semanticTable = NULL;
			LCUE_ASSERT(usageMasks <= (uint32_t const*)shaderBinaryInfo);
			usedApiSlotCount = 0;
			if (shaderStage == Gnm::kShaderStageVs)
			{
				usedApiSlotCount = ((Gnmx::VsShader*)gnmxShaderStruct)->m_numInputSemantics;
				semanticTable = ((Gnmx::VsShader*)gnmxShaderStruct)->getInputSemanticTable();
			}
			else if (shaderStage == Gnm::kShaderStageLs)
			{
				usedApiSlotCount = ((Gnmx::LsShader*)gnmxShaderStruct)->m_numInputSemantics;
				semanticTable = ((Gnmx::LsShader*)gnmxShaderStruct)->getInputSemanticTable();
			}
			else if (shaderStage == Gnm::kShaderStageEs)
			{
				usedApiSlotCount = ((Gnmx::EsShader*)gnmxShaderStruct)->m_numInputSemantics;
				semanticTable = ((Gnmx::EsShader*)gnmxShaderStruct)->getInputSemanticTable();
			}
			// Check if the shader uses any API-slot over the maximum count configured to be handled by the LCUE
			LCUE_ASSERT(usedApiSlotCount > 0 && usedApiSlotCount <= LCUE::kMaxVertexBufferCount);

			for (uint8_t j=0; j<usedApiSlotCount; j++)
			{
				int32_t semanticIndex = semanticTable[j].m_semantic;
				LCUE_ASSERT(semanticIndex >= 0 && semanticIndex < LCUE::kMaxVertexBufferCount);
				outTable->vertexBufferDwOffset[semanticIndex] = requiredMemorySizeInDwords + semanticIndex * Gnm::kDwordSizeVertexBuffer;
			}
			requiredMemorySizeInDwords += (semanticTable[usedApiSlotCount-1].m_semantic+1) * Gnm::kDwordSizeVertexBuffer;
			
		}
		break;
		}
	}
	outTable->requiredBufferSizeInDwords = requiredMemorySizeInDwords;

	// Checking for unhandled input data
	for (int32_t i=0; i<inputUsageSlotsCount; ++i)
	{
		switch (inputUsageSlots[i].m_usageType)
		{
		case Gnm::kShaderInputUsageImmResource:
		case Gnm::kShaderInputUsageImmRwResource:
		case Gnm::kShaderInputUsageImmSampler:
		case Gnm::kShaderInputUsageImmConstBuffer:
		case Gnm::kShaderInputUsageImmVertexBuffer:
		case Gnm::kShaderInputUsageImmShaderResourceTable:
		case Gnm::kShaderInputUsageSubPtrFetchShader:
		case Gnm::kShaderInputUsagePtrExtendedUserData:
		case Gnm::kShaderInputUsagePtrResourceTable:
		case Gnm::kShaderInputUsagePtrRwResourceTable:
		case Gnm::kShaderInputUsagePtrConstBufferTable:
		case Gnm::kShaderInputUsagePtrVertexBufferTable:
		case Gnm::kShaderInputUsagePtrSamplerTable:
		case Gnm::kShaderInputUsagePtrInternalGlobalTable:
		case Gnm::kShaderInputUsageImmGdsCounterRange:
		//case Gnm::kShaderInputUsagePtrSoBufferTable:		// Only present in the VS copy-shader that doesn't have a footer
			// Handled
			break;

		default:
			// Not handled yet
			LCUE_ASSERT(false);
			break;
		}
	}
	
	return 0;
}