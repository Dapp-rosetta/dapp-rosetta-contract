- Ch 1 分钱
    - 1.1 A life of a penny
    - 1.2 Ethereum 实现
        - 1.2.1 Ethereum 区块链浏览器
        - 1.2.2 Ethereum 账户系统
        - 1.2.3 使用 Remix IDE 编写 Solidity 合约
        - 1.2.4 Ethereum Gas 简介 
    - 1.3 EOS 实现
        - 1.3.1 EOS 区块链浏览器
        - 1.3.2 EOS 账户系统
        - 1.3.3 EOS 的消息分发机制
        - 1.3.4 使用 eosio.cdt 编写 C++ 合约        
        - 1.3.5 EOS 中的稀缺资源
    - 章节注记


# Ch 1 分钱
智能合约，也叫智能合同，它是一段公开可验证的运行在区块链上的代码。
现实世界中经常发生双方合作完成一项外包订单的，客户按照约定给双方打钱的情况，
本章介绍的分钱合约，只需要客户将货款打向这个合约，就可以自动完成向双方的转账。
正是可以帮助双方在无需第三方担保的情况下，实现去中心化担保，起到类似合同的作用。
通过分钱合约，我们来了解看如何编写一个简单的无状态合约，
在 Ethereum 和 EOS 合约中监听和实施转账。

## 1.1 A life of a penny
交易是比特币系统中最重要的部分。根据比特币系统的设计原理，系统中任何其他的部分都是为了确保比特币交易可以被生成、能在比特币网络中得以传播和通过验证，并最终添加入全球比特币交易总账簿（比特币区块链）。比特币交易的本质是数据结构，这些数据结构中含有比特币交易参与者价值转移的相关信息。比特币区块链是一本全球复式记账总账簿，每个比特币交易都是在比特币区块链上的一个公开记录。

例如下图是著名的比特币解密游戏，比特币挑战的官方钱包，
下图是包含大奖 310 个比特币的转账。
https://blockexplorer.com/address/39uAUwEFDi5bBbdBm5ViD8sxDBBrz7SUP4
https://bitcoinchallenge.codes/

遗憾的是，比特币中并不支持图灵完备性，因此谜题设计者最终将直接将钱包的私钥作为谜底，
而无法设计更加复杂的交互逻辑。

## 1.2 Ethereum 实现
不同于比特币的 UTXO 模型，以太坊为了更好的实现智能合约，采取了有状态的设计。
以太坊中的状态分为两类，由用户私钥控制的钱包账户，和用代码控制的合约账户。

https://medium.com/@ConsenSys/thoughts-on-utxo-by-vitalik-buterin-2bb782c67e53


*** 1.2.1 Ethereum 区块链浏览器

*** 1.2.1 Ethereum 账户系统

*** 1.2.3 使用 Remix IDE 编写 Solidity 合约
我们使用 Remix 编写和调试本节的合约。Remix 是官方推荐的 Web 端智能合约开发 IDE，
代码开源，无需安装任何依赖，适合在浏览器中快速部署和验证智能合约。

https://remix.ethereum.org
https://github.com/ethereum/remix-ide

```
pragma solidity ^0.4.24;

contract Counter {
    uint256 public counter;

    event Inc(address from);

    constructor() public {

    }

    function inc() public {
        counter += 1;
        emit Inc(msg.sender);
    }

    function get() public view returns(uint256 _counter) {
        return counter;
    }
}
```