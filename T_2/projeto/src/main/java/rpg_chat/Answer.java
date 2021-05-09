package rpg_chat;

import java.util.Objects;

public class Answer {
    String address;
    String text;
    private int points = 0;
    public Answer(String address, String text){
        this.text = text;
        this.address = address;
    }
    @Override
    public boolean equals(Object o) {
        // self check
        if (this == o)
            return true;
        // null check
        if (o == null)
            return false;
        // type check and cast
        if (getClass() != o.getClass())
            return false;
        Answer person = (Answer) o;
        // field comparison
        return Objects.equals(text, person.text);
    }

    public int getPoints() {
        return points;
    }

    public void setPoints(int points) {
        this.points = points;
    }

    @Override
    public String toString(){
        return "Player: "+address+" Text: "+text+" - points: "+points;
    }
}
