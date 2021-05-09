package gartic_in_java;

import org.jgroups.Address;

import java.io.Serializable;

public class Player implements Serializable {
    Address address;
    private int points = 0;
    private boolean hasBeenLeader;

    public Player(Address address){
        this.address = address;
    }

    public int getPoints() {
        return points;
    }

    public void setPoints(int points) {
        this.points = points;
    }

    public void addPoints(int points) {
        this.points = this.points + points;
    }

    public void setIsLeader() {
        this.hasBeenLeader = true;
    }

    public boolean hasBeenLeader() {
        return hasBeenLeader;
    }

    @Override
    public String toString(){
        return "Address:"+address+" - Points:"+points+" - "+" hasBeenLeader"+hasBeenLeader;
    }

    public String toSerializable() {
        return address+"|"+points+"|"+hasBeenLeader;
    }

    public void resetLeader() {
        this.hasBeenLeader = false;
    }
}
