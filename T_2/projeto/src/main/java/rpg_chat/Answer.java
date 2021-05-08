package rpg_chat;

import java.util.Objects;

public class Answer {
    String address;
    String text;
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
}
