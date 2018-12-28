Ch 1 分钱
    1.1 A life of a penny
    1.2 Ethereum 实现
        1.1.1 Ethereum 区块链浏览器
        1.1.2 Ethereum 账户系统
        1.1.3 使用 Remix IDE 编写 Solidity 合约
        1.1.4 Ethereum Gas 简介 
    1.3 EOS 实现
        1.1.1 EOS 区块链浏览器
        1.1.2 EOS 账户系统
        1.1.3 EOS 的消息分发机制
        1.1.4 使用 eosio.cdt 编写 C++ 合约        
        1.1.5 EOS 中的稀缺资源
小结

* Ch 1 分钱

本章介绍分钱合约。通过使用这个合约，任意个人或组织之间可以实现无需信任第三方，就可以实现大家都提前约定好的分钱算法。通过介绍分钱合约，我们来一窥看如何编写一个简单的无状态合约，在 Ethereum 和 EOS 合约中监听和实施转账。

** 1.1 A life of a penny
交易是比特币系统中最重要的部分。根据比特币系统的设计原理，系统中任何其他的部分都是为了确保比特币交易可以被生成、能在比特币网络中得以传播和通过验证，并最终添加入全球比特币交易总账簿（比特币区块链）。比特币交易的本质是数据结构，这些数据结构中含有比特币交易参与者价值转移的相关信息。比特币区块链是一本全球复式记账总账簿，每个比特币交易都是在比特币区块链上的一个公开记录。

例如下图是著名的比特币解密游戏，比特币挑战的官方钱包，
下图是包含大奖 310 个比特币的转账。
https://blockexplorer.com/address/39uAUwEFDi5bBbdBm5ViD8sxDBBrz7SUP4
https://bitcoinchallenge.codes/

** 1.2 Ethereum 实现
不同于比特币的 UTXO 模型，以太坊为了更好的实现智能合约，采取了有状态的设计。
以太坊中的状态分为两类，由用户私钥控制的钱包账户，和用代码控制的合约账户。

https://medium.com/@ConsenSys/thoughts-on-utxo-by-vitalik-buterin-2bb782c67e53