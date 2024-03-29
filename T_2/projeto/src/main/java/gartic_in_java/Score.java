package gartic_in_java;

import org.jgroups.Address;

import java.io.Serializable;
import java.util.*;
import java.util.concurrent.atomic.AtomicBoolean;

public class Score implements Serializable {
    // one could add only a integer as value of the hashmap,
    // but this Player is intended to be used for custom configuration for each player
    Map<String, Player> players = new HashMap();
    Random R = new Random();

    public void resetScore(){
        if(players.isEmpty()){
            return;
        }
        players.forEach(((s, player) -> {
            player.setPoints(0);
            player.resetLeader();
        }));
    }

    public void addPlayer(Address address){
        players.putIfAbsent(address.toString(), new Player(address));
        System.out.println("Player added ->"+players.get(address.toString()).toString());
    }

    public void updateScore(String address, int points){
        Player player = players.get(address);
        player.addPoints(points);
        players.put(address, player);
    }

    @Override
    public String toString(){
        final String[] result = {""};
        players.forEach(((s, player)->{
            result[0] +=  player.toString() + "\n";
        }));
        return result[0];
    }
    private void resetLeadership(){
        // Verify if every player have been leader
        AtomicBoolean allLeader = new AtomicBoolean(true);
        players.forEach((s, player) -> {
            allLeader.set(allLeader.get() && player.hasBeenLeader());
        });
        if(allLeader.get()){
            // if every player has been leader, reset the property so the sort of new leader can work.
            players.forEach(((s, player) -> {
                player.resetLeader();
            }));
        }
    }

    public Address electNewLeader() {
        Address newLeader = null;
        // Reset leadership of all players when everyone has already played
        resetLeadership();

        while (newLeader == null){
            Player proposedLeader = sortLeader();
            if(!proposedLeader.hasBeenLeader()){
                newLeader = proposedLeader.address;
            }
        }

        return newLeader;
    }

    private Player sortLeader() {
        return (Player) players.values().toArray()[R.nextInt(players.size())];
    }

    public void updateLeader(Address address) {
        players.get(address.toString()).setIsLeader();
    }

    public String toSerializable(){
        final String[] result = {""};
        players.forEach(((s, player)->{
            result[0] +=  player.toSerializable()+":";
        }));
        System.out.println("Score to serializable:"+result[0]);
        return result[0];
    }

    // I've decided to do this as Address is not serializable and it is used in Player class.
    public void updateScoreByString(String serializableStr){
        String[] playersStr = serializableStr.split(":");
        for(int i = 0 ; i < playersStr.length; i++){// -1 because the last index is empty, as ":" is the end of the string
            String[] params = playersStr[i].split(";");
            Player player = this.players.get(params[0]);
            if(player != null) {
                player.setPoints(Integer.parseInt(params[1]));
                if (Boolean.valueOf(params[2])) {
                    player.setIsLeader();
                }
                players.put(params[0], player);
            }
        }
    }
}
