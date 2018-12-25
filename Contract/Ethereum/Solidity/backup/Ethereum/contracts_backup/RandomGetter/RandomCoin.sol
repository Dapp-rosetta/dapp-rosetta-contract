pragma solidity ^0.4.17;

contract RandomCoin {
    uint256 constant BLOCK_DELAY = 2;
    uint256 constant BET_EXPIRATION_BLOCKS = 100;

    enum GameId {
        CoinFlip,
        SingleDice,
        DoubleDice,

        MaxGameId
    }
    // Struct is tightly packed into a single 256-bit by Solidity compiler.
    // This is made to reduce gas costs of placing & settlement transactions.
    struct ActiveBet {
        // A game that was played.
        GameId gameId;
        // Block number in which bet transaction was mined.
        uint56 placeBlockNumber;
        // A binary mask with 1 for each option.
        // For example, if you play dice, the mask ranges from 000001 in binary (betting on one)
        // to 111111 in binary (betting on all dice outcomes at once).
        uint64 mask;
        // Bet amount in wei.
        uint128 amount;
    }

    mapping (address => ActiveBet) activeBets;
    
    function settleBet(address gambler) public view returns(uint256){
        ActiveBet storage bet = activeBets[msg.sender];

        // require (bet.amount != 0);
        require (block.number > bet.placeBlockNumber + BLOCK_DELAY);
        require (block.number <= bet.placeBlockNumber + BET_EXPIRATION_BLOCKS);

        bytes32 entropy = keccak256(gambler, block.blockhash(bet.placeBlockNumber + BLOCK_DELAY));
        uint256 dice = uint256(entropy) % 2;
        return dice;
    }
}