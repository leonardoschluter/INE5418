package rpg_chat;
import org.jgroups.*;
import org.jgroups.demos.Chat;
import org.jgroups.demos.Draw;
import org.jgroups.stack.IpAddress;
import org.jgroups.util.ExtendedUUID;
import org.jgroups.util.Util;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class SimpleChat implements Receiver {
    JChannel channel;
    Integer leader = null;
    String wordToDraw = "";
    Queue<Answer> roundHistory = new LinkedList<Answer>();
    String user_name=System.getProperty("user.name", "n/a");
    Integer ID = null;
    final List<String> state=new LinkedList<String>();
    private MyDraw draw;

    // MESSAGES
    static final String MSG_warningStartRound = " ||| WARNING ||| The round will start in few seconds ... ";
    static final String MSG_gotItRight = "Congratulations, you did get it right !";
    static final String MSG_wordToDraw = "WORD TO DRAW:";
    static final String MSG_roundStarted = "ROUND STARTED:";

    public void viewAccepted(View new_view) {
        System.out.println("** view: " + new_view);
    }

    public void receive(Message msg) {
        String line=msg.getObject();
        if(line.equals(MSG_warningStartRound) || line.equals(MSG_gotItRight) || line.equals(MSG_roundStarted)){
            // TODO tratar mensagen MSG_roundStarted differently
            System.out.println(line);
        }
        String[] answer =  line.split(":");
        if(isLeader() && wordToDraw.equals(answer[1])){
            System.out.println("Submitted Answer:" + answer[1] + " - Addr: "+ answer[0]);
            checkWord(answer);
        }
        synchronized(state) {
            state.add(line);
        }
    }

    public void getState(OutputStream output) throws Exception {
        synchronized(state) {
            Util.objectToStream(state, new DataOutputStream(output));
        }
    }

    @SuppressWarnings("unchecked")
    public void setState(InputStream input) throws Exception {
        List<String> list=(List<String>)Util.objectFromStream(new DataInputStream(input));
        synchronized(state) {
            state.clear();
            state.addAll(list);
        }

    }

    private void checkWord(String[] answer) {
        roundHistory.add(new Answer(answer[0], answer[1]));
        List<Address> addresses  = channel.getView().getMembers().stream().filter(address -> {
            return address.toString().equals(answer[0]);
        }).collect(Collectors.toList());
        if(!addresses.isEmpty() && addresses.get(0) != null) {
            Message msg = new ObjectMessage(addresses.get(0), MSG_gotItRight);
            try {
                channel.send(msg);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Allways first joined node is master. This mehod prints message only once.
     */
    private boolean isLeader() {
        if(leader == null){
            leader = 0 ;
        }
        Address leaderAddress = channel.getView().getMembers().get(leader);
        Address currentAddress = channel.getAddress();
        return leaderAddress.equals(currentAddress);
    }

    private void start() throws Exception {
        channel= new JChannel("/home/leonardo/Documentos/devTools/jgroups/udp.xml");
        channel.setReceiver(this);
        channel.connect("ChatCluster");
        Thread thread = new Thread(){
            public void run(){
                try {
                    dispatchDrawApp();
                } catch (Exception e) {
                    System.out.println("Error: did not start draw app");
                    e.printStackTrace();
                }
            }
        };
        thread.start();
        eventLoop();

        thread.join();
        channel.close();
    }

    private void dispatchDrawApp() throws Exception {
        JChannel drawChannel = new JChannel("/home/leonardo/Documentos/devTools/jgroups/udp.xml");
        this.draw = new MyDraw(drawChannel, isLeader());
        this.draw.setClusterName("DrawCluster");
        this.draw.go();
        drawChannel.close();
    }
    private void eventLoop() {
        BufferedReader in=new BufferedReader(new InputStreamReader(System.in));
        while(true) {
            try {
                System.out.print("> "); System.out.flush();
                String line=in.readLine().toLowerCase();
                if(line.startsWith("quit") || line.startsWith("exit")) {
                    break;
                }
                if(line.equals("/start") && wordToDraw.isEmpty() && isLeader() ){
                    startRound();
                }else {
                    line = channel.getAddressAsString() + ":" + line;
                    Message msg = new ObjectMessage(null, line);
                    channel.send(msg);
                }
            }
            catch(Exception e) {
            }
        }
    }

    private void startRound() throws Exception {

        Message msg = new ObjectMessage(null, MSG_warningStartRound);
        channel.send(msg);
        wordToDraw = sortNextWord();
        System.out.println(MSG_wordToDraw+wordToDraw);
        Thread.sleep(1000);
        draw.clearPanel();
        Date ts = new Date();
        msg = new ObjectMessage(null, MSG_roundStarted+ts.getTime());
        channel.send(msg);
        try{
            wait(1000*90);
        }catch (InterruptedException e ){
            e.printStackTrace();
        }
    }

    private String sortNextWord() {
        return "hardcoded";
    }

    public static void main(String[] args) throws Exception {
        new SimpleChat().start();
    }
}