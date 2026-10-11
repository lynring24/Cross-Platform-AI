## 질문 1. — 03~04장

Queue가 정확히 무엇인가요? Queue와 QueueFamily의 관계는?

## 질문 2. — 05장

버퍼 생성, 메모리 할당, `bindMemory()`는 각각 무엇을 하나요?

## 질문 3. — 05장

`HostVisible`, `HostCoherent`, `DeviceLocal`은 각각 무엇을 보장하나요?
`HostVisible`과 `DeviceLocal`을 동시에 가질 수 있나요?

## 질문 4. — 05장

다음 세 줄이 각각 하는 일을 설명해주세요.
```
void* mapped = memory.mapMemory(0, bufferSize);
std::memcpy(mapped, values.data(), bufferSize);
memory.unmapMemory();
```
- `unmapMemory()` 이후에도 기록한 데이터가 남아 있나요?
- `HostCoherent`이면 GPU가 계산한 결과를 CPU에서 언제든 읽어도 되나요?

## 질문 5. — 06~07장

다음 설정은 각각 무엇을 선언하나요? 어느 것끼리 일치해야 하고, 어느 것은 달라도 되나요?

```
버퍼 생성의 usage
레이아웃 binding의 descriptorType
풀의 poolSize.type
갱신 요청의 write.descriptorType
```

## 질문 6. — 06~07장

다음 각 상황에서 **buffers, descriptor set, descriptor pool, descriptor set layout**을 각각 어떻게 처리해야 할지 설명해주세요.

**상황1**

A·B 버퍼에 담긴 숫자를 더해 C 버퍼에 결과를 저장했습니다. 이제 A·B에 새로운 숫자를 넣고 같은 계산을 다시 실행하려 합니다.

데이터의 개수와 자료형은 같으며, 기존 버퍼의 같은 영역을 사용합니다. 무엇을 변경해야 하나요?

**상황2**

A·B를 더하는 대신, 이미 만들어져 있는 A′ 버퍼와 B 버퍼를 더해 C에 저장하려 합니다. A′의 크기와 자료형은 A와 같습니다.

기존 descriptor set에는 A·B·C가 연결되어 있습니다. 앞으로 A를 사용할 필요는 없으며, 기존 연결 정보를 보존하지 않아도 됩니다. 새 descriptor set을 할당해야 할까요? 어떻게 처리할 수 있나요?

**상황3**

다음 두 계산을 번갈아 실행하려 합니다.

```
C₀ = A₀ + B₀
C₁ = A₁ + B₁
```

여섯 버퍼는 모두 준비되어 있지만, descriptor set은 A₀·B₀·C₀를 연결한 하나만 있습니다. 두 계산은 같은 셰이더와 같은 binding 구조를 사용합니다.

**두 묶음의 연결 정보를 모두 보관하고, 계산을 바꿀 때마다 descriptor set의 내용을 다시 쓰지 않으려 합니다.** 기존 pool에는 할당 여유가 있습니다.

무엇을 추가로 준비해야 하며, 실행할 때는 무엇을 선택해야 하나요?

**상황4**

하나의 pool을 다음 용량으로 만들었습니다.

```
최대 descriptor set 수: 3
StorageBuffer 디스크립터 총용량: 6
```

이 pool에서 set 두 개를 할당했고, 각 set에는 입력 둘과 출력 하나를 위한 StorageBuffer 디스크립터가 3개씩 들어 있습니다.

이제 같은 구조의 세 번째 입력·출력 묶음을 추가하려 합니다. **기존 set 두 개는 그대로 보관해야 하며**, 새 묶음의 버퍼들은 이미 준비되어 있습니다.

기존 pool에서 set을 하나 더 할당할 수 있나요? 할 수 없다면 무엇을 새로 준비해야 하나요? 기존 layout도 다시 만들어야 하나요?

**상황5**

기존 셰이더는 다음 구조를 사용합니다.

```
binding 0: StorageBuffer — A
binding 1: StorageBuffer — B
binding 2: StorageBuffer — C
```

새 셰이더는 여기에 계산 설정값을 받는 binding을 추가로 요구합니다.

```
binding 3: UniformBuffer — 계산 설정값
```

설정값을 담은 버퍼는 이미 준비되어 있고, 기존 pool에는 새 set을 할당할 공간과 필요한 두 타입의 디스크립터 용량이 충분합니다.

기존 descriptor set에 write만 한 번 더 하면 될까요? 기존 layout과 set을 그대로 사용할 수 있는지, 무엇을 새로 만들어야 하는지 설명해주세요. (Pipeline 설정은 제외합니다) 
